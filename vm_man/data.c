/********** INCLUDE FILES **********/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../util/clist.h"
#include "../util/cerror.h"
#include "../util/xallocate.h"
#include "vm_man.h"
#include "data.h"

/********** PRIVATE FUNCTIONS **********/

static int cmp(void * data, const char * name){
	assert(data != NULL);
	VMinfo_p vm = (VMinfo_p)data;

	if(!strcmp(vm->vmname, name)) {
		return SUCCESS;
	}
	else {
		return FAIL;
	}
}

/********** GLOBAL FUNCTIONS **********/

int add_vm(PMinfo_p pm, VMinfo_p vm){
	
	assert(pm != NULL);
	assert(pm->vms != NULL);

	if(SUCCESS != clInsertTop(pm->vms, (void *)vm)){
		return FAIL;
	}

	if(vm->state == VIR_DOMAIN_RUNNING || vm->state == VIR_DOMAIN_BLOCKED
						|| vm->state == VIR_DOMAIN_PAUSED || vm->state == VIR_DOMAIN_SHUTOFF){
		pm->avail_memory -= vm->mem;
		pm->avail_cpu -= vm->cpu;
	}
	
	return SUCCESS;
}

VMinfo_p allocate_vm(PMinfo_p pm, const char * vmname, int state, unsigned long mem, int cpu){

	VMinfo_p vm = xmalloc(sizeof(VMinfo));
	
	if(vmname){
		strncpy(vm->vmname, vmname, NAME_LEN);
	}
	vm->state = state;
	vm->host_pm = pm;
	vm->mem = mem;
	vm->cpu = cpu;

	return vm;
}

VMinfo_p find_vm(const char * vmname){
	PMinfo_p pnext = NULL;
	VMinfo_p next = NULL;

	clReset(global_pms);
	while((pnext = (PMinfo_p)clGetNext(global_pms)) != NULL){
		if(pnext->vms != NULL){
			
			clReset(pnext->vms);
			while((next = (VMinfo_p)clGetNext(pnext->vms)) != NULL){
				if(!strcmp(next->vmname, vmname)) return next;
			}
		}
	}

	return NULL;
}

int remove_vm(const char * vmname){
	PMinfo_p pnext = NULL;

	clReset(global_pms);
	while((pnext = (PMinfo_p)clGetNext(global_pms)) != NULL){
		if(pnext->vms != NULL && clRemove(pnext->vms, cmp, vmname)){
			return SUCCESS;
		}
	}

	return SUCCESS;
}

int print_vms(const PMinfo_p pm){
	VMinfo_p next = NULL;

	if(!pm->vms) return SUCCESS;

	clReset(pm->vms);
	while((next = (VMinfo_p)clGetNext(pm->vms)) != NULL){
		printf("%-18s", next->vmname);
		switch(next->state){
			case 0: printf("no state     ");break;
			case 1: printf("running      ");break;
			case 2: printf("blocked      ");break;
			case 3: printf("paused       ");break;
			case 4: printf("shutdown     ");break;
			case 5: printf("shutoff      ");break;
			case 6: printf("crashed      ");break;
			default: printf("unknow state ");
		}
		printf("%8lumb", next->mem);
		printf("%5dvcpus\n", next->cpu);
	}

	return SUCCESS;
}

int print_all_vms(){
    PMinfo_p next;
	printf("-----------------------------------------------------------\n");
	printf("                        VM information \n");
	printf("-----------------------------------------------------------\n");
	
	clReset(global_pms);
	while((next = (PMinfo_p)clGetNext(global_pms)) != NULL){
		print_vms(next);
	}

	return SUCCESS;
}
