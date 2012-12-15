/********** include files **********/

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "cerror.h"

/********** global functions **********/

inline void * xmalloc(size_t size){
	void * ret = NULL;

	if(size == 0){
		CERROR("Cannot allocate buffer of size 0.\n");
		return NULL;
	}

	ret = (void *)malloc(size);
	if(!ret){
		CERROR("lib-malloc error: %s\n", strerror(errno));
		return NULL;
	}
	memset(ret, 0, size);

	return ret;
}

inline void * xrealloc(void * ptr, size_t size){
	void * ret = realloc(ptr, size);
	if(!ret){
		CERROR("lib-realloc error: %s\n", strerror(errno));
		return NULL;
	}

	return ret;
}

inline void doXfree(void * ptr){
	if(ptr){
		free(ptr);
	}
}
