#include "config.h"

typedef int (*gettimeofday_t)(struct timeval *tv, struct timezone *tz);
typedef int (*clock_gettime_t)(clockid_t clk_id, struct timespec *tp);

gettimeofday_t gettimeofday_REAL = NULL;
clock_gettime_t clock_gettime_REAL = NULL;

uint64_t time_div = 1;
uint64_t time_mul = 1;
double time_speedhack = 1.0f;

int initd_hack = 0;
struct timeval  gtod_time;
struct timeval  gtod_base;
struct timespec clkgt_m_time;
/* CLOCK_REALTIME
   CLOCK_REALTIME_COARSE
   CLOCK_MONOTONIC
   CLOCK_MONOTONIC_COARSE
   CLOCK_BOOTTIME
   CLOCK_PROCESS_CPUTIME_ID
   CLOCK_THREAD_CPUTIME_ID */
struct timespec clkgt_m_base;

int mode_to_int(int mode) {
	switch(mode) {
		case CLOCK_REALTIME:
			return 0;
		case CLOCK_REALTIME_COARSE:
			return 1;
		case CLOCK_MONOTONIC:
			return 2;
		case CLOCK_MONOTONIC_COARSE:
			return 3;
		case CLOCK_BOOTTIME:
			return 4;
		case CLOCK_PROCESS_CPUTIME_ID:
			return 5;
		case CLOCK_THREAD_CPUTIME_ID:
			return 6;
		default:
			return -1;
	}
}

int int_to_mode(int mode) {
	switch(mode) {
		case 0:
			return CLOCK_REALTIME;
		case 1:
			return CLOCK_REALTIME_COARSE;
		case 2:
			return CLOCK_MONOTONIC;
		case 3:
			return CLOCK_MONOTONIC_COARSE;
		case 4:
			return CLOCK_BOOTTIME;
		case 5:
			return CLOCK_PROCESS_CPUTIME_ID;
		case 6:
			return CLOCK_THREAD_CPUTIME_ID;
	}
}

// Init state.
void clock_hack_init() {
	check_cfg_mod();

	gettimeofday_REAL = dlsym(RTLD_NEXT, "gettimeofday");
	clock_gettime_REAL = dlsym(RTLD_NEXT, "clock_gettime");

	if (gettimeofday_REAL(&gtod_time, NULL)) {
		fprintf(stderr, "[HOOK] !!! SANITY ERROR !!! - real gettimeofday cannot be used, aborting\n");
		exit(-1);
	}

	gtod_base.tv_sec  = gtod_time.tv_sec;
	gtod_base.tv_usec = gtod_time.tv_usec;

	if (clock_gettime_REAL(CLOCK_MONOTONIC, &clkgt_m_time)) {
		fprintf(stderr, "[HOOK] !!! SANITY ERROR !!! - real gettimeofday cannot be used, aborting\n");
		exit(-1);
	}

	clkgt_m_base.tv_sec  = clkgt_m_time.tv_sec;
	clkgt_m_base.tv_nsec = clkgt_m_time.tv_nsec;

	if (!ENABLE_TIME_HACKS)
		fprintf(stderr, "[HOOK] Speedhack is disabled.\n", time_speedhack);
	else
		fprintf(stderr, "[HOOK] Speedhack is applied. Time factor is %lf.\n", time_speedhack);

	initd_hack = 1;
}

// Transform time based on fraction.
void xform_timeval(struct timeval *tv, struct timeval *orig) {
	// What is this? Well, on init we save the initial point. From there,
	// time moves at half-speed. So if I run a game at 2:00 PM, it's not back in 1990.

	// PROBLEM - Time changes. We add a different value for the 'base'.
	tv->tv_sec  = gtod_base.tv_sec  + ((tv->tv_sec  - orig->tv_sec)  * SLOW_FACTOR);
	tv->tv_usec = gtod_base.tv_usec + ((tv->tv_usec - orig->tv_usec) * SLOW_FACTOR);

}

void xform_timespec(struct timespec *tv, struct timespec *orig) {
	tv->tv_sec  = clkgt_m_base.tv_sec  + ((tv->tv_sec  - orig->tv_sec)  * SLOW_FACTOR);
	tv->tv_nsec = clkgt_m_base.tv_nsec + ((tv->tv_nsec - orig->tv_nsec) * SLOW_FACTOR);
}

int done_gt_print = 0;

// Fake gettimeofday.
int gettimeofday(struct timeval *tv, struct timezone *tz) {
	if (!initd_hack) clock_hack_init();

	int r = gettimeofday_REAL(tv, NULL);
	if (ENABLE_TIME_HACKS && ENABLE_GETTIMEOFDAY_HOOK)
		xform_timeval(tv, &gtod_time);

	if (check_cfg_mod()) {
		gtod_base.tv_sec  = tv->tv_sec;
		gtod_base.tv_usec = tv->tv_usec;
	}

	if (tv != NULL && tv->tv_sec % 5 == 0) {
		if (!done_gt_print) printf("[gettimeofday]  %llu\t%llu\n", tv->tv_sec, tv->tv_usec);
		done_gt_print = 1;
	} else
		done_gt_print = 0;

	return r;
}

int done_clkgt_print = 0;

// Fake clock_gettime.
int clock_gettime(clockid_t clk_id, struct timespec *tp) {
	if (!initd_hack) clock_hack_init();

	int r = clock_gettime_REAL(clk_id, tp);
	if (ENABLE_TIME_HACKS && ENABLE_CLOCK_GETTIME_HOOK)
		xform_timespec(tp, &clkgt_m_time);

	if (check_cfg_mod()) {
		clkgt_m_base.tv_sec  = tp->tv_sec;
		clkgt_m_base.tv_nsec = tp->tv_nsec;
	}

	if (tp != NULL && tp->tv_sec % 5 == 0) {
		mode_to_int(clk_id);
		if (!done_clkgt_print) printf("[clock_gettime] %llu\t%llu\n", tp->tv_sec, tp->tv_nsec);
		done_clkgt_print = 1;
	} else
		done_clkgt_print = 0;

	return r;
}
