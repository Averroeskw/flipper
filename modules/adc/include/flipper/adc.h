#ifndef __adc_h__
#define __adc_h__

/* Include all types and macros exposed by the Flipper Toolbox. */
#include <flipper/core.h>

/* Declare the virtual interface for this module. */
extern const struct _adc {
	void (* configure)(void);
} adc;

#ifdef __private_include__

/* Declare the FMR overlay for this driver. */
enum { _adc_configure };

/* Declare each prototype for all functions within this driver. */
void adc_configure(void);

#endif
#endif