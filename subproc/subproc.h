#ifndef __NGKH_SUBPROC_H__
#define __NGKH_SUBPROC_H__

#include <stdio.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

pid_t ngkh_subproc(char *const *argv, FILE **std_in, FILE **std_out);

#ifdef __cplusplus
}
#endif

#endif

