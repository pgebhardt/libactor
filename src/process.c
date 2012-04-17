#include <stdlib.h>
#include <dispatch/dispatch.h>
#include "process.h"

// spawn new process
process_process* process_spawn(node_node* node, process_process_function function) {
    // TODO
    // create message queue
    message_queue* queue = message_queue_create(node->serial_queue);

    // create process struct
    process_process* process = malloc(sizeof(process_process));
    process->queue = queue;
    process->pid = 0;

    // call process function
    dispatch_async(node->concurrent_queue, ^(void) {
            function(process);
        });

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
