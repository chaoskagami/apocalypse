#define DEFINED_ENABLES_HERE

#include <dlfcn.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <stdint.h>

#include <sys/time.h>
#include <sys/stat.h>
#include <time.h>

#include "config.h"

int ENABLE_TIME_HACKS = 0;
int ENABLE_GETTIMEOFDAY_HOOK = 0;
int ENABLE_CLOCK_GETTIME_HOOK = 0;

// Is this Isaac: AB or RB?
int IS_TBOI = 0;

double SLOW_FACTOR = 1.0;

int HAS_LOADED_CONFIG = 0;

struct stat info;

void read_cfg() {
	FILE* f = fopen("apocalypse.conf", "r");
	if (!f) {
		// Doesn't exist, so write the example.
		fprintf(stderr, "[HOOK] No config file found, writing one out to 'apocalypse.conf'\n");
		f = fopen("apocalypse.conf", "w");
		fprintf(f,
			"#================================\n"
			"# Apocalypse tool config file\n"
			"#================================\n\n"
			"# Enable time hacks. Required for slowdown\n"
			"# and speedup, as well.\n"
			"time_hacks=0\n"
			"# Enable gettimeofday-based time hack.\n"
			"gettimeofday=0\n"
			"# Enable clock_gettime-based time hack.\n"
			"clock_gettime=0\n"
			"# Slow factor with time hack.\n"
			"slow_factor=1.0\n\n"
			"#================================\n"
			"# Game-specific stuff\n"
			"#================================\n\n"
			"# Binding of isaac hooks\n"
			"isaac=0\n"
		);
		fclose(f);
		f = fopen("apocalypse.conf", "r");
	}

	stat("apocalypse.conf", &info);

	char* line = NULL;
	size_t len = 0;
	int read = 0;

	while ((read = getline(&line, &len, f)) != -1) {
		// Line less than three characters? Next.
		if (strlen(line) < 3)
			continue;

		// Line is \n? Next.
		if (line[0] == '\n')
			continue;

		// First character is #? Next.
		if (line[0] == '#')
			continue;

		char *prop, *value;

		prop = line;

		int i;
		for(i=0; i < strlen(line); i++) {
			if (line[i] == '=') {
				line[i] = '\0';
				value = &line[i+1];
			}
		}

		if ( !strcmp(prop, "time_hacks") ) {
			sscanf(value, "%d", & ENABLE_TIME_HACKS);
			fprintf(stderr, "[HOOK] time_hacks: %d\n", ENABLE_CLOCK_GETTIME_HOOK);
		}
		else if ( !strcmp(prop, "gettimeofday") ) {
			sscanf(value, "%d", & ENABLE_GETTIMEOFDAY_HOOK);
			fprintf(stderr, "[HOOK] gettimeofday: %d\n", ENABLE_GETTIMEOFDAY_HOOK);
		}
		else if ( !strcmp(prop, "clock_gettime") ) {
			sscanf(value, "%d", & ENABLE_CLOCK_GETTIME_HOOK);
			fprintf(stderr, "[HOOK] clock_gettime: %d\n", ENABLE_CLOCK_GETTIME_HOOK);
		}
		else if ( !strcmp(prop, "slow_factor") ) {
			sscanf(value, "%lf", & SLOW_FACTOR);
			fprintf(stderr, "[HOOK] slow factor: %lf\n", SLOW_FACTOR);
		}
	}

	fprintf(stderr, "[HOOK] Read 'apocalypse.conf'. Settings loaded.\n");

	fclose(f);

	HAS_LOADED_CONFIG = 1;
}

int check_cfg_mod() {
	struct stat new;
	stat("apocalypse.conf", &new);
	if (new.st_mtim.tv_sec != info.st_mtim.tv_sec || new.st_mtim.tv_nsec != info.st_mtim.tv_nsec) {
		// File was modified. Reload.
		read_cfg();
		memcpy(&info, &new, sizeof(struct stat));
		return 1;
	}
	return 0;
}
