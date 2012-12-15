/********** include files **********/

#include "cerror.h"
#include <stdarg.h>

/********** global functions **********/

int CERROR(const char * fmt, ...){
	va_list ap;
	char printf_buf[1024];
	int re_len = 0;

#ifdef NDEBUG
	va_start(ap, fmt);
	re_len = vsprintf(printf_buf, fmt, ap);
	va_end(ap);
	puts(printf_buf);
#endif

	return re_len;
}
