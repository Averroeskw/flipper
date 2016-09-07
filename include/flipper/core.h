/* core.h - Exposes all types and macros provided by the Flipper Toolbox. */

#ifndef __lf_core_h__
#define __lf_core_h__

/* Include the standard library headers. */
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Define USB related constants. */
#define USB_VENDOR_ID	0x16C0
#define USB_PRODUCT_ID	0x0480
#define USB_USAGE_PAGE	0xFFAB
#define USB_USAGE		0x0200

/* Used to contain the result of checksumming operations. */
typedef uint16_t lf_id_t;
/* Describes a type used to contain libflipper error codes. */
typedef uint32_t lf_error_t;
/* Describes a pointer to an address within non-volatile memory. */
typedef uint32_t nvm_p;

/* Used to quantify block sizes sent accross different platforms. */
typedef uint32_t lf_size_t;
#define LF_SIZE_T_MAX UINT32_MAX

#define lf_success 0
#define lf_error -1

/* Computes the greatest integer from the result of the division of x by y. */
#define lf_ceiling(x, y) ((x + y - 1) / y)

/* Macros that quantify device attributes. */
#define lf_device_8bit (fmr_int8_t << 1)
#define lf_device_16bit (fmr_int16_t << 1)
#define lf_device_32bit (fmr_int32_t << 1)
#define lf_device_big_endian 1
#define lf_device_little_endian 0

/* Define bit manipulation macros. */
#define bit(b)								(0x01 << (b))
#define get_bit_from_port(b, p)				((p) & bit(b))
#define set_bit_in_port(b, p)				((p) |= bit(b))
#define set_bits_in_port_with_mask(p, m)	((p) |= (m))
#define clear_bit_in_port(b, p)				((p) &= ~(bit(b)))
#define clear_bits_in_port_with_mask(p, m)	((p) &= ~(m))
#define flip_bit_in_port(b, p)				((p) ^= bit(b))
#define flip_bits_in_port_with_mask(p, m)	((p) ^= (m))
#define lo(x) ((uint8_t)(x))
#define hi(x) ((uint8_t)(x >> 8))

/* Standardizes interaction with a physical hardware bus for the transmission of arbitrary data. */
struct _lf_endpoint {
	/* Configures the endpoint hardware given an arbitrary parameter. */
	int (* configure)();
	/* Indicates whether or not the hardware is ready to send or receive data. */
	uint8_t (* ready)(void);
	/* Sends a single byte through the endpoint. */
	void (* put)(uint8_t byte);
	/* Retrieves a single byte from the endpoint. */
	uint8_t (* get)(void);
	/* Transmits a block of data through the endpoint. */
	int (* push)(void *source, lf_size_t length);
	/* Receives a block of data from the endpoint. */
	int (* pull)(void *destination, lf_size_t length);
	/* Destroys any state associated with the endpoint. */
	int (* destroy)();
	/* The only state associated with an endpoint; tracks endpoint specific configuration information. */
	void *record;
};

/* Describes a device capible of responding to FMR packets. */
struct _lf_device {
	/* The human readable name of the device. */
	char *name;
	/* An identifier unique to the device. */
	lf_id_t identifier;
	/* The attributes of the device. 2:1 (word length), 0 (endianness) */
	uint8_t attributes;
	/* A pointer to the endpoint through which packets will be transferred. */
	const struct _lf_endpoint *endpoint;
	/* The current error state of the device. */
	lf_error_t error;
	/* Describes whether or not errors generated on this device will produce side effects on the host. */
	uint8_t errors_cause_exit;
	/* The next device in the list of attached devices. */
	struct _lf_device *next;
};

/* Provides a checksum for a given block of data. */
lf_id_t lf_checksum(void *source, size_t length);

#endif