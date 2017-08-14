#ifndef __NGKH_SUBPROC_H__
#define __NGKH_SUBPROC_H__

#include <stdio.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

pid_t ngkh_subproc(const char *exec_path, char *const *argv, FILE **std_in, FILE **std_out);
char *ngkh_subproc_find_exec_path(const char *exec_name); // Return string must be free after using.

#ifdef __cplusplus
}
#endif

#endif

