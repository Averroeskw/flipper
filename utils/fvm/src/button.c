#include <flipper/libflipper.h>

#ifdef __use_button__
#define __private_include__
#include <flipper/button.h>

int button_configure(void) {
	printf("Configuring the button.\n");
	return lf_success;
}

uint8_t button_read(void) {
	printf("Reading the state of the button.\n");
	return 0;
}

#endif
