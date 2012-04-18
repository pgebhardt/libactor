#include <stdlib.h>
#include "node.h"
#include "process.h"

// message sendig
message_message* process_message_send(process_process* process, process_id dest_id,
    message_message* message) {
    // check for valid process
    if (process == NULL) {
        return NULL;
    }

    // check for correct dest_id
    if (dest_id >= process->node->process_size) {
        return NULL;
    }

    // get destination message queue
    message_queue* dest_queue = node_message_queue_get(process->node, dest_id);

    // check for succes
    if (dest_queue == NULL) {
        return NULL;
    }

    // enqueue message
    message_queue_put(dest_queue, message);

    return message;
}

// message receive
message_message* process_message_receive(process_process* process,
    double timeout) {
    // check for correct input
    if ((process == NULL) || (timeout < 0.0)) {
        return NULL;
    }

    // get message
    return message_queue_get(process->queue, timeout);
}

// process cleanup
void process_cleanup(process_process* process) {
    // check for valid process
    if (process == NULL) {
        return;
    }

    // release queue
    node_message_queue_release(process->node, process->pid);

    // release dipatch queue
    dispatch_release(process->dispatch_queue);
}

void process_release(process_process* process) {
    // check for valid process
    if (process == NULL) {
        return;
    }

    // cleanup process
    process_cleanup(process);

    // free process memory
    free(process);
}
