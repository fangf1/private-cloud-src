/********** INCLUDE FILES **********/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm_man.h"
#include "data.h"
#include "../pm_man/pm_man.h"
#include "../util/cerror.h"
#include "../util/xallocate.h"

/********** PRIVATE FUNCTIONS **********/

static void vm_destroy(void *ptr){
    VMinfo_p temp = (VMinfo_p)ptr;

	temp->host_pm->avail_memory += temp->mem;
	temp->host_pm->avail_cpu += temp->cpu;
	xfree(ptr);
}

static virConnectPtr getConnect(PMinfo_p pm){
	if(pm->conn == NULL){
		pm->conn = virConnectOpen(pm->url);
		if (pm->conn == NULL) {
			fprintf(stderr, "Failed to open connection to xen:/// at node %s\n", pm->ip);
			return NULL;
		}
	}
	
	return pm->conn;
}

/********** GLOBAL FUNCTIONS **********/

int init_vms(clist * vms){
	
	*vms = clInit(vm_destroy);

	return SUCCESS;
}

VMinfo_p VMShowstate(const char * vmname){
	return find_vm(vmname);
}

clist VMShowStateInPM(PMinfo_p p){
	if(p){
		print_vms(p);
		return p->vms;
	}
	else{
		printf("no this pm\n");
		return NULL;
	}
}

int VMDestroy(const char * vmname){
	VMinfo_p vm;
	virDomainPtr dom;
	int re;

	vm = find_vm(vmname);
	if(!vm) return FAIL;

	dom = virDomainLookupByName(getConnect(vm->host_pm), vmname);
	//printf("%s\n", virDomainGetName(dom));
	if (dom == NULL){
		CERROR("Failed to find Domain %s\n", vmname);
		return FAIL;
	}
	re = virDomainDestroy(dom);		
	if (re == -1) return FAIL;
	re = virDomainUndefine(dom);
	virDomainFree(dom);
	if(re == -1) return FAIL;
	else {
		re = remove_vm(vmname);
		return SUCCESS;
	}
}

int VMMigrate(const char * vmname, PMinfo_p pm){
	VMinfo_p vm = NULL;
	virDomainPtr dom, after_dom;
	char cmd[1024];

	vm = find_vm(vmname);
	if(!vm || !pm) return FAIL;
	if(!(vm->state == VIR_DOMAIN_RUNNING || vm->state == VIR_DOMAIN_BLOCKED)) return FAIL;
	if(pm == vm->host_pm) return SUCCESS;

	dom = virDomainLookupByName(getConnect(vm->host_pm), vmname);
	if (dom == NULL){
		fprintf(stderr, "Failed to find Domain %s\n", vmname);
		return FAIL;
	}

	printf("wait some seconds to migrate\n");
	after_dom = virDomainMigrate(dom, getConnect(pm), VIR_MIGRATE_LIVE, NULL, pm->ip, 0);
	if(after_dom) {
		snprintf(cmd, 1024, "scp %s:/etc/xen/%s .", vm->host_pm->ip, vmname);
		system(cmd);
		snprintf(cmd, 1024, "scp %s %s:/etc/xen", vmname, pm->ip);
		system(cmd);
		snprintf(cmd, 1024, "rm %s", vmname);
		system(cmd);
		virDomainFree(dom);
		VMinfo_p new_vm = allocate_vm(pm, vm->vmname, vm->state, vm->mem, vm->cpu);
		PMinfo_p old_pm = vm->host_pm;
		remove_vm(vm->vmname);

		add_vm(pm, new_vm);
		dom = virDomainLookupByName(getConnect(old_pm), vmname);
		if(virDomainUndefine(dom) < 0) return FAIL;
		virDomainFree(dom);
		virDomainFree(after_dom);
		return SUCCESS;
	}
	virDomainFree(dom);
	virDomainFree(after_dom);
	return FAIL;
}

virDomainPtr VMDefineXML(PMinfo_p pm, char * filename){
	virDomainPtr dom;
	FILE * fp;
	char xml[1024];
	int i = 0;
	char c;

	memset(xml, '\0', 1024);

    if((fp = fopen(filename, "r")) == NULL){
		fprintf(stderr, "Failed to open XML file\n");
		return NULL;
	}

//	while((c = fgetc(fp)) != EOF){
//	    xml[i++] = c;
//	}
	while(fread(&c, sizeof(char), 1, fp) == 1){
		xml[i++] = c;
	}
	fclose(fp);
//	printf("%s", xml);

	virConnectPtr conn = getConnect(pm);
//	virConnectPtr conn = virConnectOpen(pm->url);
	dom = virDomainDefineXML(conn, xml);

	return dom;
}

int VMCreate(PMinfo_p pm, char * filename){
	virDomainPtr dom = VMDefineXML(pm, filename);
        
	if (virDomainCreate(dom) < 0){
		fprintf(stderr, "Failed to create new vm\n");
		virDomainFree(dom);
		return FAIL;
	}

	VMinfo_p vm;
	virDomainInfo info;
	virDomainGetInfo(dom, &info);
	if((vm = allocate_vm(pm, virDomainGetName(dom), 1, info.maxMem/1024UL, info.nrVirtCpu)) == NULL)
		fprintf(stderr, "Failed to allocate vm\n");
    if(add_vm(pm, vm) != SUCCESS)
		fprintf(stderr, "Failed to add vm\n");
	virDomainFree(dom);   

    printf("New VM has been created successfully!\n");

	return SUCCESS;
}

int VMStart(const char * vmname){
	VMinfo_p vm = find_vm(vmname);
    if(vm == NULL){
	    fprintf(stderr, "No such VM %s found\n", vmname);
	    return FAIL;
	}

	if(vm->state != VIR_DOMAIN_PAUSED && vm->state != VIR_DOMAIN_SHUTOFF ){
	    fprintf(stderr, "VM %s's state error, can not be started\n", vmname);
	    return FAIL;
	}

	virDomainPtr dom = virDomainLookupByName(getConnect(vm->host_pm), vmname);
	if (dom == NULL){
	    fprintf(stderr, "Failed to find Domain %s\n", vmname);
	    return FAIL;
	}

	if((vm->state == VIR_DOMAIN_PAUSED && (virDomainResume(dom) < 0))
			||(vm->state == VIR_DOMAIN_SHUTOFF && (virDomainCreate(dom) < 0))){
	    fprintf(stderr, "Failed to start Domain %s\n", vmname);
    	virDomainFree(dom);
    	return FAIL;
	}
	virDomainFree(dom);

	vm->state = VIR_DOMAIN_RUNNING;

	printf("Vm %s has been started successfully!\n", vmname);
	
	return SUCCESS;
}

int VMPause(const char * vmname){
	VMinfo_p vm = find_vm(vmname);
	if(vm == NULL){
		fprintf(stderr, "No such VM %s found\n", vmname);
		return FAIL;
	}
/*
    if(vm->state != '1'){
	    fprintf(stderr, "VM state error, %s can not be paused\n", vmname);
	    return 1;
	}
*/
	virDomainPtr dom = virDomainLookupByName(getConnect(vm->host_pm), vmname);
	if (dom == NULL){
	    fprintf(stderr, "Failed to find Domain %s\n", vmname);
	    return FAIL;
	}

	if (virDomainSuspend(dom) < 0){
	    fprintf(stderr, "Failed to pause Domain %s\n", vmname);
	    virDomainFree(dom);
	    return FAIL;
	}
	virDomainFree(dom);
	
	vm->state = VIR_DOMAIN_PAUSED;

	printf("Vm %s has been paused successfully!\n", vmname);

	return SUCCESS;
}

int VMRestart(const char * vmname){
	VMinfo_p vm = find_vm(vmname);
	if(vm == NULL){
		fprintf(stderr, "No such VM %s found\n", vmname);
		return FAIL;
	}

	/*
	if(vm->state != '1'){
		fprintf(stderr, "VM state error, %s can not be restarted\n", vmname);
		return 1;
	}
*/
	virDomainPtr dom = virDomainLookupByName(getConnect(vm->host_pm), vmname);
	if (dom == NULL){
		fprintf(stderr, "Failed to find Domain %s\n", vmname);
		return FAIL;
	}
	
	if (virDomainReboot(dom, 0) < 0){
		fprintf(stderr, "Failed to reboot Domain %s\n", vmname);
		virDomainFree(dom);
		return FAIL;
	}
	virDomainFree(dom);
	
	printf("Vm %s has been restarted successfully!\n", vmname);
	
	return SUCCESS;
}

int VMShutdown(const char * vmname){
    VMinfo_p vm = find_vm(vmname);
	if(vm == NULL){
		fprintf(stderr, "No such VM %s found\n", vmname);
		return FAIL;
    }

/*  if(vm->state != '1'){
        fprintf(stderr, "VM state error, %s can not be shut down\n", vmname);
        return 1;
	}	
 */
	virDomainPtr dom = virDomainLookupByName(getConnect(vm->host_pm), vmname);
	if (dom == NULL){
	    fprintf(stderr, "Failed to find Domain %s\n", vmname);
	    return FAIL;
	}

	if (virDomainShutdown(dom) < 0){
	    fprintf(stderr, "Failed to shut down Domain %s\n", vmname);
	    virDomainFree(dom);
	    return FAIL;
	}
	virDomainFree(dom);

	vm->state = VIR_DOMAIN_SHUTOFF;
		
    printf("Vm %s has been shut down successfully!\n", vmname);

	return SUCCESS;
}
