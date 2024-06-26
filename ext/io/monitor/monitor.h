#pragma once

#include <stddef.h>
#include <sys/types.h>

struct IO_Monitor {
	float latency;
	
	size_t size;
	const char **paths;
};

ssize_t IO_Monitor_find(struct IO_Monitor *monitor, const char *path);
void IO_Monitor_watch(struct IO_Monitor *monitor);
