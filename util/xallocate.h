/********** include files **********/

#ifndef __XALLOCATE_H__
#define __XALLOCATE_H__

#include <stddef.h>

/********** global macro declaration **********/

#define xfree(ptr) doXfree(ptr); (ptr) = NULL;

/********** global function declaration **********/

void * xmalloc(size_t size);
void * xrelloc(void * ptr, size_t size);
void doXfree(void * ptr);

#endif
