#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <dispatch/dispatch.h>
#include <stdio.h>
#include <stdlib.h>

enum {
	DEBUG = 0,
};

struct IO_FSEvent_Monitor {
	size_t size;
	const char **paths;
};

int path_prefix(const char *path, const char *prefix) {
	size_t path_size = strlen(path);
	size_t prefix_size = strlen(prefix);
	
	if (path_size < prefix_size) {
		return 0;
	}
	
	return strncmp(path, prefix, prefix_size) == 0;
}

ssize_t IO_FSEvent_Monitor_find(struct IO_FSEvent_Monitor *monitor, const char *path) {
	size_t index = 0;
	
	while (index < monitor->size) {
		if (path_prefix(path, monitor->paths[index])) {
			return index;
		}
		
		index += 1;
	}
	
	return -1;
}

// Function to handle filesystem events
void IO_FSEvent_callback(
	ConstFSEventStreamRef streamRef,
	void *context,
	size_t numberOfEvents,
	void *eventData,
	const FSEventStreamEventFlags eventFlags[],
	const FSEventStreamEventId eventIds[]) {
	
	const char **eventPaths = (const char**)eventData;
	struct IO_FSEvent_Monitor *monitor = (struct IO_FSEvent_Monitor*)context;
	
	for (size_t i = 0; i < numberOfEvents; i++) {
		if (DEBUG) fprintf(stderr, "Event: %s\n", eventPaths[i]);
		
		// Find the index of the path in the paths array
		ssize_t index = IO_FSEvent_Monitor_find(monitor, eventPaths[i]);
		
		if (index != -1) {
			// Output event data as newline-delimited JSON
			printf("{\"index\":%zu,\"flags\":%u,\"id\":%llu}\n", index, eventFlags[i], eventIds[i]);
		} else {
			fprintf(stderr, "Path not found in paths array: %s\n", eventPaths[i]);
		}
	}
}

int IO_FSEvent_watch(struct IO_FSEvent_Monitor *monitor) {
	// Prepare the array of paths to watch
	CFStringRef *pathsToWatch = malloc(sizeof(CFStringRef) * monitor->size);
	
	for (size_t i = 0; i < monitor->size; i++) {
		pathsToWatch[i] = CFStringCreateWithCString(NULL, monitor->paths[i], kCFStringEncodingUTF8);
	}
	
	CFArrayRef pathsToWatchArray = CFArrayCreate(NULL, (const void **)pathsToWatch, monitor->size, &kCFTypeArrayCallBacks);
	
	FSEventStreamContext context = {
		.version = 0,
		.info = monitor,
	};
	
	// Set up the FSEvent stream
	FSEventStreamRef stream;
	CFAbsoluteTime latency = 1.0; // Latency in seconds
	
	char * io_fsevent_latency = getenv("IO_FSEVENT_LATENCY");
	if (io_fsevent_latency != NULL) {
		latency = atof(io_fsevent_latency);
	}
	
	// Create the stream
	stream = FSEventStreamCreate(NULL,
		&IO_FSEvent_callback,
		&context,
		pathsToWatchArray,
		kFSEventStreamEventIdSinceNow,
		latency,
		kFSEventStreamCreateFlagNone
	);
	
	// Set up the dispatch queue
	dispatch_queue_t queue = dispatch_queue_create("com.example.fsevents.queue", NULL);

	// Attach the FSEvent stream to the dispatch queue
	FSEventStreamSetDispatchQueue(stream, queue);
	FSEventStreamStart(stream);

	// Keep the program running
	dispatch_main();

	// Clean up
	FSEventStreamStop(stream);
	FSEventStreamInvalidate(stream);
	FSEventStreamRelease(stream);
	for (int i = 0; i < monitor->size; i++) {
		CFRelease(pathsToWatch[i]);
	}
	free(pathsToWatch);
	CFRelease(pathsToWatchArray);
}

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <directory_to_watch> [<directory_to_watch> ...]\n", argv[0]);
		return 1;
	}
	
	argv += 1;
	argc -= 1;
	
	char * realPaths[argc];
	for (int i = 0; i < argc; i++) {
		realPaths[i] = realpath(argv[i], NULL);
		
		if (realPaths[i] == NULL) {
			fprintf(stderr, "Error: realpath failed for %s\n", argv[i]);
			return 1;
		} else {
			if (DEBUG) fprintf(stderr, "Watching: %s\n", realPaths[i]);
		}
	}
	
	struct IO_FSEvent_Monitor monitor = {
		.size = argc,
		.paths = (const char **)realPaths,
	};
	
	IO_FSEvent_watch(&monitor);
	
	return 0;
}
