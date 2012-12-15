/*===============================================================================
                                 INCLUDE FILES
================================================================================*/

#include "xallocate.h"
#include "clist.h"
#include "cerror.h"
#include <assert.h>
/*================================================================================
                                GLOBAL  FUNCTIONS
================================================================================*/

clist clInit(clistDestroyFunc destroy){
	clist ptr = NULL;
	ptr = xmalloc(sizeof(struct _clist));
	if (!ptr) 
	{
		return NULL;
	}
	ptr->size = 0;
	ptr->list = NULL;
	ptr->save = NULL;
	ptr->destroy = destroy;
	return ptr;
}

int clInsertTop(clist ref, void *data){
	struct clistnode *ret = NULL;
	
	ret = xmalloc(sizeof(struct clistnode));
	if (ret == NULL){
		return FAIL;
	}

	assert(ref != NULL);
	ret->next = NULL;
	if (data){
		ret->data = data;
	} 
	else{
		ret->data = NULL;
	}

	ret->next = ref->list;
	ref->list = ret;

	/* Reset the "save" pointer to point to the new node. */
	ref->save = ref->list;
	ref->size++;	
	
	return SUCCESS;
}

int clInsertEnd(clist ref, void *data){
	struct clistnode *current;
	struct clistnode *ret = NULL;

	ret = xmalloc(sizeof(struct clistnode));
	if (NULL == ret){
		return FAIL;
	}

	assert(ref != NULL);
	ret->next = NULL;
	if (data){
		ret->data = data;
	} 
	else{
		ret->data = NULL;
	}
	
	for(current = ref->list; current->next; current = current->next)
		;
	
	current->next = ret;
	ref->size++;

	return SUCCESS;
}

int clCopy(clist to, clist from){

	struct clistnode * current = NULL;

	for(current = from->list; current; current = current->next)	{
		if (SUCCESS != clInsertTop(to, current->data)){
			return FAIL;
		}
	}

	return SUCCESS;
}

void clReset(clist ref){
	ref->save = ref->list;
}

void * clGetNext(clist ref)
{
	void *data = NULL;
	if (ref){
		if (ref->save){
			data = ref->save->data;
			ref->save = ref->save->next;
		}
		else{
			data = NULL;
			ref->save = ref->list;
		}
	}
	return data;
}

void clDestroy(clist ref)
{
	struct clistnode *tmp = NULL;
	struct clistnode *first = ref->list;

	for (; first; first = tmp){
		tmp = first->next;
		if (first->data && ref->destroy){
			ref->destroy(first->data);
		}
		xfree(first);
	}
	xfree(ref);
}

void clPop(clist ref){
	struct clistnode *hold=NULL, *tmp=ref->list;

	hold = tmp->next;
	if (ref->destroy && tmp->data){
		ref->destroy(tmp->data);
	}
	xfree(tmp);
	ref->list = hold;
	ref->save = ref->list;
	ref->size--;
}

void * clGetTop(clist ref){
	return ref->list->data;
}

int clRemove(clist ref, clistCompare cmp, const char * name){
	struct clistnode * tmp = NULL;
	struct clistnode * pre = NULL;

	for(tmp = ref->list; tmp; tmp = tmp->next){
		if(cmp(tmp->data, name) == SUCCESS){
			if(pre){
				pre->next = tmp->next;
			}
			else{
				ref->list = tmp->next;
			}

			ref->destroy(tmp->data);
			xfree(tmp);
			ref->save = ref->list;
			ref->size--;
			return SUCCESS;
		}
		pre = tmp;
	}
	
	return FAIL;
}
