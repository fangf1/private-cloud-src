#ifndef VM_MAN_H
#define VM_MAN_H

/********** INCLUDE FILES **********/

#include <libvirt/virterror.h>
#include <libvirt/libvirt.h>
#include "../util/clist.h"
#include "../pm_man/pm_man.h"

/********** DEFINE CONSTANT VAR **********/

#define NAME_LEN 64
#define FILE_NOT_EXIST -2
#define ALLOCATE_FAIL -3

/********** DEFINE GLOBAL STRUCTURES **********/

typedef struct VMinfo{
	char vmname[NAME_LEN];
	unsigned char state;
	struct PMinfo * host_pm;
	unsigned long mem; /*MB*/
	int cpu;
}VMinfo, *VMinfo_p;

/********** DECLARE GLOBAL FUNCTIONS **********/
int init_vms();
//show vms state
VMinfo_p VMShowstate(const char * vmname);
clist VMShowStateInPM(PMinfo_p pm);

int VMCreate(PMinfo_p pm, char * filename);
//start a vm
int VMStart(const char * vmname);
//pause a vm
int VMPause(const char * vmname);
//restart a vm
int VMRestart(const char * vmname);
//shutdown a vm
int VMShutdown(const char * vmname);
//destroy a vm
int VMDestroy(const char * vmname);
//migrate a vm
int VMMigrate(const char * vmname, PMinfo_p pm);

#endif
