#include "monitor.h"

#include <sys/inotify.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

enum {
	DEBUG = 0,
};

#define BUFFER_SIZE (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

void IO_Monitor_watch(struct IO_Monitor *monitor) {
	int fd = inotify_init1(IN_NONBLOCK);
	if (fd == -1) {
		perror("inotify_init1");
		exit(EXIT_FAILURE);
	}

	int *wd = malloc(sizeof(int) * monitor->size);
	if (!wd) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	for (size_t i = 0; i < monitor->size; i++) {
		wd[i] = inotify_add_watch(fd, monitor->paths[i], IN_ALL_EVENTS);
		if (wd[i] == -1) {
			perror("inotify_add_watch");
			exit(EXIT_FAILURE);
		}
	}

	char buffer[BUFFER_SIZE] __attribute__ ((aligned(8)));
	ssize_t len, i;

	while (1) {
		len = read(fd, buffer, BUFFER_SIZE);
		if (len == -1 && errno != EAGAIN) {
			perror("read");
			exit(EXIT_FAILURE);
		}

		for (i = 0; i < len;) {
			struct inotify_event *event = (struct inotify_event *) &buf[i];

			printf("{\"wd\":%d,\"mask\":%u,\"name\":\"%s\"}\n",
				   event->wd, event->mask, event->len ? event->name : "");

			i += sizeof(struct inotify_event) + event->len;
		}
		fflush(stdout);
	}

	for (size_t i = 0; i < monitor->size; i++) {
		inotify_rm_watch(fd, wd[i]);
	}
	close(fd);
	free(wd);
}