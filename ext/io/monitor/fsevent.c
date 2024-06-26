#include "monitor.h"

#include <CoreServices/CoreServices.h>
#include <dispatch/dispatch.h>
#include <stdio.h>

enum {
	DEBUG = 0,
};

// Function to handle filesystem events
void IO_Monitor_FSEvent_callback(
	ConstFSEventStreamRef streamRef,
	void *context,
	size_t numberOfEvents,
	void *eventData,
	const FSEventStreamEventFlags eventFlags[],
	const FSEventStreamEventId eventIds[]) {
	
	const char **eventPaths = (const char**)eventData;
	struct IO_Monitor *monitor = context;
	
	for (size_t i = 0; i < numberOfEvents; i++) {
		if (DEBUG) fprintf(stderr, "Event: %s\n", eventPaths[i]);
		
		// Find the index of the path in the paths array
		ssize_t index = IO_Monitor_find(monitor, eventPaths[i]);
		
		if (index != -1) {
			// Output event data as newline-delimited JSON
			printf("{\"index\":%zu,\"flags\":%u,\"id\":%llu}\n", index, eventFlags[i], eventIds[i]);
		} else {
			fprintf(stderr, "Path not found in paths array: %s\n", eventPaths[i]);
		}
	}
	
	fflush(stdout);
}

void IO_Monitor_watch(struct IO_Monitor *monitor) {
	CFStringRef *pathsToWatch = malloc(sizeof(CFStringRef) * monitor->size);
	for (size_t i = 0; i < monitor->size; i++) {
		pathsToWatch[i] = CFStringCreateWithCString(NULL, monitor->paths[i], kCFStringEncodingUTF8);
	}
	
	CFArrayRef pathsToWatchArray = CFArrayCreate(NULL, (const void **)pathsToWatch, monitor->size, &kCFTypeArrayCallBacks);
	
	FSEventStreamContext context = {0, monitor, NULL, NULL, NULL};
	
	FSEventStreamRef stream;
	CFAbsoluteTime latency = monitor->latency;
	
	stream = FSEventStreamCreate(
		NULL,
		&IO_Monitor_FSEvent_callback,
		&context,
		pathsToWatchArray,
		kFSEventStreamEventIdSinceNow,
		latency,
		kFSEventStreamCreateFlagNone
	);
	
	dispatch_queue_t queue = dispatch_queue_create("com.example.fsevents.queue", NULL);
	FSEventStreamSetDispatchQueue(stream, queue);
	FSEventStreamStart(stream);
	
	printf("{\"status\":\"started\"}\n");
	fflush(stdout);
	
	dispatch_main();
	
	FSEventStreamStop(stream);
	FSEventStreamInvalidate(stream);
	FSEventStreamRelease(stream);
	for (size_t i = 0; i < monitor->size; i++) {
		CFRelease(pathsToWatch[i]);
	}
	free(pathsToWatch);
	CFRelease(pathsToWatchArray);
}