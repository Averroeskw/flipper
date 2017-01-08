#define __private_include__
#include <flipper/carbon/platforms/atmega16u2.h>
#include <megausb.h>
#include <flipper/carbon/modules.h>

/* The fmr_device object containing global state about this device. */
struct _lf_device lf_self = {
	{
		"flipper",
		0xc713,
		LF_VERSION,
		(lf_device_16bit | lf_device_little_endian)
	},
	&megausb,
	E_OK
};

#define cpu_prescale(clock) (CLKPR = 0x80, CLKPR = clock)

void system_task(void) {
	while (1) {
		struct _fmr_packet packet;
		int8_t _e = megausb_pull(&megausb, (void *)(&packet), sizeof(struct _fmr_packet));
		if (!(_e < lf_success)) {
			/* Create a buffer to the result of the operation. */
			struct _fmr_result result;
			/* Execute the packet. */
			fmr_perform(&packet, &result);
			/* Send the result back to the host. */
			megausb_push(&megausb, &result, sizeof(struct _fmr_result));
			/* Clear any error state generated by the procedure. */
			error_clear();
		}
		/* Pet the watchdog. */
		//wdt_reset();
	}
}

void fmr_push(struct _fmr_push_pull_packet *packet) {
	void *swap = malloc(packet -> length);
	if (!swap) {
		error_raise(E_MALLOC, NULL);
		return;
	}
	lf_self.endpoint -> pull(lf_self.endpoint, swap, packet -> length);
	*(uintptr_t *)(packet -> call.parameters) = (uintptr_t)swap;
	fmr_execute(packet -> call.index, packet -> call.function, packet -> call.argc, packet -> call.types, (void *)(packet -> call.parameters));
	free(swap);
}

void fmr_pull(struct _fmr_push_pull_packet *packet) {
	void *swap = malloc(packet -> length);
	if (!swap) {
		error_raise(E_MALLOC, NULL);
		return;
	}
	*(uintptr_t *)(packet -> call.parameters) = (uintptr_t)swap;
	fmr_execute(packet -> call.index, packet -> call.function, packet -> call.argc, packet -> call.types, (void *)(packet -> call.parameters));
	lf_self.endpoint -> push(lf_self.endpoint, swap, packet -> length);
	free(swap);
}

void system_init() {
	/* Prescale CPU for maximum clock. */
	cpu_prescale(0);
	/* Configure the USB subsystem. */
	configure_usb();
	/* Configure the UART0 subsystem. */
	uart0_configure();
	/* Configure the SAM4S. */
	cpu_configure();
	/* Enable the watchdog timer. */
	//wdt_enable(WDTO_500MS);
	/* Configure reset button and PCINT8 interrupt. */
//	PCMSK1 |= (1 << PCINT8);
//	PCICR |= (1 << PCIE1);
	/* Globally enable interrupts. */
	sei();
}

void system_deinit(void) {
	/* Clear the state of the status LED. */
	led_set_rgb(LED_OFF);
}

void system_reset(void) {
	/* Shutdown the system. */
	system_deinit();
	/* Fire the watchdog timer. */
	wdt_fire();
	/* Wait until reset. */
	while (1);
}

/* PCINT8 interrupt service routine; captures reset button press and resets the device. */
ISR (PCINT1_vect) {
	system_reset();
}