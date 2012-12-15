/********** INCLUDE FILES **********/

#include "pm_man.h"
#include "data.h"
#include "../util/cerror.h"
#include "../util/xallocate.h"

/********** GLOBAL VARIBLES **********/

clist global_pms = NULL;

/********** PRIVATE FUNCTIONS **********/

static void pm_destroy(void *ptr){
	PMinfo_p temp = (PMinfo_p)ptr;

	clDestroy(temp->vms);
	xfree(temp->hostname);
    xfree(ptr);
}

/********** GLOBAL FUNCTIONS **********/

int init_pms(){
	global_pms = clInit(pm_destroy);
	
	return SUCCESS;
}

