#include <stdlib.h>
#include <dispatch/dispatch.h>
#include "process.h"
#include "message.h"

// spawn new process
process_process* process_spawn(process_process_function function) {
    // TODO
    // get concurrent queue
    dispatch_queue_t concurrentQueue = dispatch_get_global_queue(
        DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);

    // create message queue
    message_queue* queue = message_queue_create(concurrentQueue, 100);

    // call process function
    dispatch_async(concurrentQueue, ^(void) {
            function();
        });

    // create process struct
    process_process* process = malloc(sizeof(process_process));
    process->queue = queue;
    process->pid = 0;

    // return error pid
    return process;
}

// process cleanup
void process_cleanup(process_process* process) {
    // cleanup message queue
    message_queue_cleanup(process->queue);

    // free process memory
    free(process);
}
