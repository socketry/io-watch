#include "monitor.h"

#include <sys/inotify.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

enum {
	DEBUG = 0,
};

struct IO_Monitor_Watch {
	int watch_descriptor;
	char *path;
	int index;
};

struct IO_Monitor_Watch_Array {
	size_t size;
	size_t capacity;
	struct IO_Monitor_Watch *watches;
};

#define BUFFER_SIZE (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

void IO_Monitor_Watch_Array_initialize(struct IO_Monitor_Watch_Array *array) {
	array->size = 0;
	array->capacity = 16;
	array->watches = malloc(array->capacity * sizeof(struct IO_Monitor_Watch));
	if (!array->watches) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
}

void IO_Monitor_Watch_Array_resize(struct IO_Monitor_Watch_Array *array) {
	array->capacity *= 2;
	array->watches = realloc(array->watches, array->capacity * sizeof(struct IO_Monitor_Watch));
	if (!array->watches) {
		perror("realloc");
		exit(EXIT_FAILURE);
	}
}

void IO_Monitor_Watch_Array_add(struct IO_Monitor_Watch_Array *array, int watch_descriptor, const char *path, int index) {
	if (array->size == array->capacity) {
		IO_Monitor_Watch_Array_resize(array);
	}
	array->watches[array->size].watch_descriptor = watch_descriptor;
	array->watches[array->size].path = strdup(path);
	array->watches[array->size].index = index;
	if (!array->watches[array->size].path) {
		perror("strdup");
		exit(EXIT_FAILURE);
	}
	array->size++;
}

ssize_t IO_Monitor_Watch_Array_find(struct IO_Monitor_Watch_Array *array, int watch_descriptor) {
	for (size_t i = 0; i < array->size; i++) {
		if (array->watches[i].watch_descriptor == watch_descriptor) {
			return i;
		}
	}
	return -1;
}

void IO_Monitor_Watch_Array_watch(int fd, struct IO_Monitor_Watch_Array *watch_array, const char *path, int index) {
	int watch_descriptor = inotify_add_watch(fd, path, IN_ALL_EVENTS);
	if (watch_descriptor == -1) {
		perror("inotify_add_watch");
		exit(EXIT_FAILURE);
	}
	IO_Monitor_Watch_Array_add(watch_array, watch_descriptor, path, index);
	if (DEBUG) printf("Added watch: %s\n", path);
}

void IO_Monitor_Watch_Array_scan(int fd, struct IO_Monitor_Watch_Array *watch_array, const char *root, int index) {
	DIR *dir = opendir(root);
	if (!dir) {
		perror("opendir");
		return;
	}

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			continue;
		}

		char path[PATH_MAX];
		snprintf(path, sizeof(path), "%s/%s", root, entry->d_name);

		struct stat statbuf;
		if (stat(path, &statbuf) == -1) {
			perror("stat");
			continue;
		}

		if (S_ISDIR(statbuf.st_mode)) {
			IO_Monitor_Watch_Array_watch(fd, watch_array, path, index);
			IO_Monitor_Watch_Array_scan(fd, watch_array, path, index);
		}
	}

	closedir(dir);
}

void IO_Monitor_watch(struct IO_Monitor *monitor) {
	int fd = inotify_init1(IN_NONBLOCK);
	if (fd == -1) {
		perror("inotify_init1");
		exit(EXIT_FAILURE);
	}

	struct IO_Monitor_Watch_Array watch_array;
	IO_Monitor_Watch_Array_initialize(&watch_array);

	for (size_t i = 0; i < monitor->size; i++) {
		IO_Monitor_Watch_Array_watch(fd, &watch_array, monitor->paths[i], i);
		IO_Monitor_Watch_Array_scan(fd, &watch_array, monitor->paths[i], i);
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

			ssize_t index = IO_Monitor_Watch_Array_find(&watch_array, event->wd);

			if (index != -1) {
				printf("{\"index\":%d,\"mask\":%u,\"name\":\"%s\"}\n",
					   watch_array.watches[index].index, event->mask, event->len ? event->name : "");

				// If a new directory is created, add a watch for it
				if (event->mask & IN_CREATE && event->mask & IN_ISDIR) {
					char new_path[PATH_MAX];
					snprintf(new_path, PATH_MAX, "%s/%s", watch_array.watches[index].path, event->name);
					IO_Monitor_Watch_Array_watch(fd, &watch_array, new_path, watch_array.watches[index].index);
					IO_Monitor_Watch_Array_scan(fd, &watch_array, new_path, watch_array.watches[index].index);
				}
			} else {
				fprintf(stderr, "Watch descriptor not found: %d\n", event->wd);
			}

			i += sizeof(struct inotify_event) + event->len;
		}
		fflush(stdout);
	}

	for (size_t i = 0; i < watch_array.size; i++) {
		inotify_rm_watch(fd, watch_array.watches[i].watch_descriptor);
		free(watch_array.watches[i].path);
	}
	close(fd);
	free(watch_array.watches);
}