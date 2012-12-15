/********** INCLUDE FILES **********/

#ifndef VM_DATA_H
#define VM_DATA_H

/********** GLOBAL FUNCTION PROTOTYPES **********/

int add_vm(PMinfo_p pm, VMinfo_p vm);
VMinfo_p allocate_vm(PMinfo_p pm, const char * vmname, int state, unsigned long, int cpu);
int print_vms(const PMinfo_p pm);
int print_all_vms();
VMinfo_p find_vm(const char * vmname);
int remove_vm(const char * vmname);

#endif
