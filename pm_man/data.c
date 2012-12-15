/********** INCLUDE FILES **********/

#include <string.h>
#include <libvirt/virterror.h>
#include <libvirt/libvirt.h>
#include "../util/clist.h"
#include "../util/xallocate.h"
#include "../util/cerror.h"
#include "pm_man.h"
#include "data.h"

/********** PRIVATE FUNCTIONS **********/

static PMinfo_p find_pm_by_name(const char * pmname){
    PMinfo_p next = NULL;

	clReset(global_pms);
	while((next = (PMinfo_p)clGetNext(global_pms)) != NULL){
		if(!strcmp(next->hostname, pmname)) return next;
	}

	return NULL;
}

static PMinfo_p find_pm_by_ip(const char * pmip){
    PMinfo_p next = NULL;

    clReset(global_pms);
    while((next = (PMinfo_p)clGetNext(global_pms)) != NULL){
        if(!strcmp(next->ip, pmip)) return next;
    }

	return NULL;
}
/********** GLOBAL FUNCTION **********/

int add_pm(PMinfo_p pm){
	if(SUCCESS != clInsertTop(global_pms, (void *)pm)){
		return FAIL;
	}

	return SUCCESS;
}

PMinfo_p allocate_pm(const char * ip){
	virNodeInfo nodeinfo;

	PMinfo_p pm = xmalloc(sizeof(PMinfo));
	
	/* memory out error */
	if(!pm) return NULL;

	if(ip){
	   	strncpy(pm->ip, ip, 16);
		sprintf(pm->url, "xen+ssh://root@%s/", ip);

		pm->conn = virConnectOpen(pm->url);
		if(!pm->conn){
		   	fprintf(stderr, "Failed to open connection to xen:/// at node %s\n", pm->ip);
			pm->state = SHUTDOWN;
		}
		else{
			pm->state = RUNNING;
			virNodeGetInfo(pm->conn, &nodeinfo);
			pm->hostname = virConnectGetHostname(pm->conn);	
			pm->max_memory = nodeinfo.memory / 1024;
			pm->avail_memory = pm->max_memory;
			pm->max_cpu = nodeinfo.cpus * 2;
			pm->avail_cpu = pm->max_cpu;
			pm->vms = NULL;
		}
	}

	return pm;
}

PMinfo_p find_pm(const char * pm){
	PMinfo_p re;
	re = find_pm_by_name(pm);
	if(re) return re;
	re = find_pm_by_ip(pm);
	return re;
}

void print_pms(){
	PMinfo_p next;

    printf("-----------------------------------------------------------\n");
	printf("                        PM information \n");
	printf("-----------------------------------------------------------\n");

	clReset(global_pms);

	while( (next = (PMinfo_p)clGetNext(global_pms)) != NULL){
		printf("%-18s", next->hostname);
		printf("%-18s", next->ip);
		printf("%llu/%llumb", next->avail_memory, next->max_memory);
		printf("  %u/%u", next->avail_cpu, next->max_cpu);
		printf("\n");
	}
}
