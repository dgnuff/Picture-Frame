#pragma once

#if defined(_DEBUG) && !defined(DEBUG)
#define DEBUG 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern char *optarg;
extern int optind;
extern int opterr;

int getopt(int nargc, char * const *nargv, char const *ostr);

#ifdef __cplusplus
}
#endif

// For EOF
#include <stdio.h>

/*
* pragma to link in the library
*/
#ifdef _WIN64
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "clibd.lib")
#else
#pragma comment(lib, "clib.lib")
#endif
#endif
