#include <stdio.h>
#include <dispatch/dispatch.h>
#include "process.h"

// spawn new process
process_id process_spawn(process_process_function function) {
    // TODO
    // get concurrent queue
    dispatch_queue_t concurrentQueue = dispatch_get_global_queue(
        DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);

    // call process function
    dispatch_async(concurrentQueue, ^(void) {
            function();
        });

    // return error pid
    return 0;
}
