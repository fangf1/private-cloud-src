/********** INCLUDE FILES **********/

#ifndef PM_DATA_H
#define PM_DATA_H

#include "pm_man.h"

/********** GLOBAL FUNCTION PROTOTYPES **********/

int add_pm(PMinfo_p pm);
PMinfo_p allocate_pm(const char *);
void print_pms();
PMinfo_p find_pm(const char * pm);

#endif
