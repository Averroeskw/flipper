#define __private_include__
#include <flipper/flipper.h>
#include <flipper/carbon/temp.h>

int temp_configure(void) {
	return lf_invoke(&_temp, _temp_configure, NULL);
}
