#ifndef __NGKH_SUBPROC_LOCAL_H__
#define __NGKH_SUBPROC_LOCAL_H__

#include <stdbool.h>

bool ngkh_check_exist_executable(const char *dir, const char *exec_name, char *abs_path); // abs_path must have length of PATH_MAX + 1.
#endif

