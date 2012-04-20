#include <stdlib.h>
#include "node.h"
#include "process.h"

// message sendig
actor_message_t actor_message_send(actor_process_t process,
    actor_process_id_t dest_id, actor_message_t message) {
    // check for valid process
    if (process == NULL) {
        return NULL;
    }

    // check for correct dest_id
    if (dest_id >= process->node->process_size) {
        return NULL;
    }

    // get destination message queue
    actor_message_queue_t dest_queue = actor_node_message_queue_get(process->node,
        dest_id);

    // check for succes
    if (dest_queue == NULL) {
        return NULL;
    }

    // enqueue message
    actor_message_queue_put(dest_queue, message);

    return message;
}

// message receive
actor_message_t actor_message_receive(actor_process_t process,
    double timeout) {
    // check for correct input
    if ((process == NULL) || (timeout < 0.0)) {
        return NULL;
    }

    // get message
    return actor_message_queue_get(process->message_queue, timeout);
}

// process cleanup
void actor_process_cleanup(actor_process_t process) {
    // check for valid process
    if (process == NULL) {
        return;
    }

    // release queue
    actor_node_message_queue_release(process->node, process->pid);
}

void actor_process_release(actor_process_t process) {
    // check for valid process
    if (process == NULL) {
        return;
    }

    // cleanup process
    actor_process_cleanup(process);

    // free process memory
    free(process);
}
