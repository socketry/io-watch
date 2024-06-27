#pragma once

#include <stddef.h>
#include <sys/types.h>

struct IO_Monitor {
	float latency;
	
	size_t size;
	const char **paths;
};

void IO_Monitor_watch(struct IO_Monitor *monitor);
