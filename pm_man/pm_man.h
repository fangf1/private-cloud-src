#ifndef PM_MAN_H
#define PM_MAN_H

/********** INCLUDE FILES **********/

#include <libvirt/virterror.h>
#include <libvirt/libvirt.h>
#include "../util/clist.h"

/********** DEFINE CONSTANT VAR **********/

#define NAME_LEN 64

/********** DEFINE GLOBAL STRUCTURES **********/

typedef struct PMinfo{
	virConnectPtr conn;
	char url[512];
	char * hostname;
	char ip[16];
	unsigned long long max_memory; /*MB*/
	unsigned long long avail_memory; /*MB*/
	unsigned int max_cpu;
	unsigned int avail_cpu;
	int state;
	clist vms;
}PMinfo, *PMinfo_p;

//PM STATUS
enum {RUNNING, SHUTDOWN, BLOCKED, PAUSED};

/*********** DECLARE GLOBAL VARIBLE ***********/

extern clist global_pms;

/*********** DECLARE GLOBAL FUNCTION PROTOTYPES **********/
int init_pms();

#endif
