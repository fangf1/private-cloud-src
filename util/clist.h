/*================================================================================
                                 INCLUDE FILES
================================================================================*/
#ifndef __CLIST_H
#define __CLIST_H

#include <sys/types.h>

/*================================================================================
                          STRUCTURES AND OTHER TYPEDEFS
================================================================================*/

typedef void (*clistDestroyFunc) (void *data);
typedef int (*clistCompare) (void * data, const char * name);

struct clistnode {
	void *data;
	size_t size;
	struct clistnode *next;
};

typedef struct _clist {
	size_t size;
	struct clistnode *save;
	struct clistnode *list;
	clistDestroyFunc destroy;
} *clist;

/*================================================================================
                          GLOBAL FUNCTION PROTOTYPES
================================================================================*/

clist clInit(clistDestroyFunc destroy);
int clInsertTop(clist ref, void *data);
int clInsertEnd(clist ref, void *data);
int clCopy(clist src, clist dest);
void clReset(clist ref);
void *clGetNext(clist ref);
void clDestroy(clist ref);
void clistPop(clist ref);
void *clGetTop(clist ref);
int clRemove(clist ref, clistCompare cmp, const char * name);

#endif
