#include "monitor.h"

#include <sys/inotify.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

enum {
	DEBUG = 0,
};

#define BUFFER_SIZE (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

ssize_t IO_Monitor_find_watch_descriptor(struct IO_Monitor *monitor, int watch_descriptors[], int watch_descriptor) {
	size_t index = 0;
	
	while (index < monitor->size) {
		if (watch_descriptors[index] == watch_descriptor) {
			return index;
		}
		
		index += 1;
	}
	
	return -1;
}

void IO_Monitor_watch(struct IO_Monitor *monitor) {
	int fd = inotify_init1(IN_NONBLOCK);
	if (fd == -1) {
		perror("inotify_init1");
		exit(EXIT_FAILURE);
	}
	
	int watch_descriptors[monitor->size];
	
	for (size_t i = 0; i < monitor->size; i++) {
		watch_descriptors[i] = inotify_add_watch(fd, monitor->paths[i], IN_ALL_EVENTS);
		if (watch_descriptors[i] == -1) {
			perror("inotify_add_watch");
			exit(EXIT_FAILURE);
		}
	}
	
	printf("{\"status\":\"started\"}\n");
	fflush(stdout);
	
	char buffer[BUFFER_SIZE] __attribute__ ((aligned(8)));
	ssize_t len, i;

	while (1) {
		len = read(fd, buffer, BUFFER_SIZE);
		if (len == -1 && errno != EAGAIN) {
			perror("read");
			exit(EXIT_FAILURE);
		}

		for (i = 0; i < len;) {
			struct inotify_event *event = (struct inotify_event *) &buffer[i];
			
			ssize_t index = IO_Monitor_find_watch_descriptor(monitor, watch_descriptors, event->wd);
			
			if (index != -1) {
				printf("{\"index\":%zu,\"mask\":%u}\n", index, event->mask);
			} else {
				fprintf(stderr, "Path not found in paths array: %s\n", event->name);
			}
			
			i += sizeof(struct inotify_event) + event->len;
		}
		fflush(stdout);
	}

	for (size_t i = 0; i < monitor->size; i++) {
		inotify_rm_watch(fd, watch_descriptors[i]);
	}
	close(fd);
}