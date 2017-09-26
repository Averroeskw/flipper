/* libusb.c - USB endpoint wrapper using libusb. */

#define __private_include__
#include <flipper/libflipper.h>
#include <flipper/posix/libusb.h>
#include <libusb-1.0/libusb.h>

struct _lf_libusb_context {
	struct libusb_device_handle *handle;
	struct libusb_context *context;
};

int lf_libusb_configure(struct _lf_endpoint *endpoint, void *_ctx) {
	return lf_success;
}

bool lf_libusb_ready(struct _lf_endpoint *endpoint) {
	return false;
}

int lf_libusb_push(struct _lf_endpoint *endpoint, void *source, lf_size_t length) {
	struct _lf_libusb_context *context = (struct _lf_libusb_context *)endpoint->_ctx;
	int _length;
	int _e;
#ifndef __ALL_BULK__
	if (length <= INTERRUPT_OUT_SIZE) {
		_e = libusb_interrupt_transfer(context->handle, INTERRUPT_OUT_ENDPOINT, source, length, &_length, LF_USB_TIMEOUT_MS);
	} else {
#endif
		_e = libusb_bulk_transfer(context->handle, BULK_OUT_ENDPOINT, source, length, &_length, LF_USB_TIMEOUT_MS);
#ifndef __ALL_BULK__
	}
#endif
	if (_e != 0) {
		if (_e == LIBUSB_ERROR_TIMEOUT) {
			lf_error_raise(E_TIMEOUT, error_message("The transfer to the device timed out."));
		} else {
			lf_error_raise(E_COMMUNICATION, error_message("Error during libusb transfer."));
		}
		return lf_error;
	}
	lf_assert(_length == length, failure, E_COMMUNICATION, "Failed to transmit complete USB packet.");
	return lf_success;
failure:
	return lf_error;
}

int lf_libusb_pull(struct _lf_endpoint *endpoint, void *destination, lf_size_t length) {
	struct _lf_libusb_context *context = (struct _lf_libusb_context *)endpoint->_ctx;
	int _length;
	int _e;
#ifndef __ALL_BULK__
	if (length <= INTERRUPT_IN_SIZE) {
		_e = libusb_interrupt_transfer(context->handle, INTERRUPT_IN_ENDPOINT, destination, length, &_length, LF_USB_TIMEOUT_MS);
	} else {
#endif
		_e = libusb_bulk_transfer(context->handle, BULK_IN_ENDPOINT, destination, length, &_length, LF_USB_TIMEOUT_MS);
#ifndef __ALL_BULK__
	}
#endif
	if (_e != 0) {
		if (_e == LIBUSB_ERROR_TIMEOUT) {
			lf_error_raise(E_TIMEOUT, error_message("The transfer to the device timed out."));
		} else {
			lf_error_raise(E_COMMUNICATION, error_message("Error during libusb transfer."));
		}
		return lf_error;
	}
	lf_assert(_length == length, failure, E_COMMUNICATION, "Failed to transmit complete USB packet.");
	return lf_success;
failure:
	return lf_error;
}

int lf_libusb_destroy(struct _lf_endpoint *endpoint) {
	if (endpoint) {
		struct _lf_libusb_context *context = (struct _lf_libusb_context *)endpoint->_ctx;
		libusb_close(context->handle);
		libusb_exit(context->context);
	}
	return lf_success;
}

struct _lf_ll *lf_libusb_endpoints_for_vid_pid(uint16_t vid, uint16_t pid) {
	struct libusb_context *context = NULL;
	struct libusb_device **libusb_devices = NULL;
	struct _lf_ll *endpoints = NULL;
	int _e = libusb_init(&context);
	lf_assert(_e == 0, failure, E_LIBUSB, "Failed to initialize libusb. Reboot and try again.");
	/* Walk the device list until all desired devices are attached. */
	size_t device_count = libusb_get_device_list(context, &libusb_devices);
	for (size_t i = 0; i < device_count; i ++) {
		struct libusb_device *libusb_device = libusb_devices[i];
		/* Obtain the device's descriptor. */
		struct libusb_device_descriptor descriptor;
		_e = libusb_get_device_descriptor(libusb_device, &descriptor);
		lf_assert(_e == 0, failure, E_LIBUSB, "Failed to obtain descriptor for device.");
		/* Check if we have a match with the desired VID and PID. */
		if (descriptor.idVendor == vid && descriptor.idProduct == pid) {
			/* Create an new endpoint for endpoint device. */
			struct _lf_endpoint *endpoint = lf_endpoint_create(lf_libusb_configure,
															   lf_libusb_ready,
															   lf_libusb_push,
															   lf_libusb_pull,
															   lf_libusb_destroy,
															   sizeof(struct _lf_libusb_context));
			lf_assert(endpoint, release, E_NULL, "Failed to create new libusb endpoint.");
			/* Retain a reference to the libusb context and give it to the context. */
			struct _lf_libusb_context *context = (struct _lf_libusb_context *)endpoint->_ctx;
			_e = libusb_init(&(context->context));
			lf_assert(_e == 0, release, E_LIBUSB, "Failed to initialize libusb. Reboot and try again.");
			/* Open the device and give it to the context. */
			_e = libusb_open(libusb_device, &(context->handle));
			lf_assert(_e == 0, release, E_NO_DEVICE, "Could not find any devices connected via USB. Ensure that a device is connected.");
			/* Claim the device's control interface. */
			_e = libusb_claim_interface(context->handle, 0);
			lf_assert(_e == 0, release, E_LIBUSB, "Failed to claim interface on attached device. Please quit any other programs using your device.");
			/* Reset the device's USB controller. */
			//_e = libusb_reset_device(context->handle);
			lf_assert(_e == 0, release, E_LIBUSB, "Failed to reset the libusb device.");
			/* Add the device to the device list. */
			_e = lf_ll_append(&endpoints, endpoint, lf_endpoint_release);
			lf_assert(_e == 0, release, E_NULL, "Failed to attach device.");
			continue;
release:
			lf_endpoint_release(endpoint);
			break;
		}
	}
failure:
	libusb_free_device_list(libusb_devices, 1);
	libusb_exit(context);
	return endpoints;
}
