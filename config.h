#define _GNU_SOURCE
#include <dlfcn.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <stdint.h>

#include <sys/time.h>
#include <time.h>

#include <GL/gl.h>
#include <GL/glx.h>

#ifndef DEFINED_ENABLES_HERE
extern int ENABLE_TIME_HACKS;
extern int ENABLE_GETTIMEOFDAY_HOOK;
extern int ENABLE_CLOCK_GETTIME_HOOK;

extern int HAS_LOADED_CONFIG;

extern int IS_TBOI;

extern double SLOW_FACTOR;

#endif

void read_cfg();
int check_cfg_mod(); // Returns 1 if reloaded.
