#include "actor.h"

// spawn process
actor_process_id_t actor_process_spawn(actor_node_t node,
    actor_process_function_t function) {
    // check for valid node
    if (node == NULL) {
        return -1;
    }

    // get free message queue
    actor_process_id_t id = 0;
    actor_message_queue_t queue = actor_node_message_queue_get_free(node, &id);

    // check for valid queue
    if (queue == NULL) {
        return -1;
    }

    // create process
    __block actor_process_t process = actor_process_create(id, node, queue);

    // check for success
    if (process == NULL) {
        // cleanup
        actor_node_message_queue_release(node, id);

        return -1;
    }

    // create dispatch queue
    dispatch_queue_t dispatch_queue = dispatch_get_global_queue(
        DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);

    // check for success
    if (dispatch_queue == NULL) {
        // cleanup
        actor_process_release(process);

        return -1;
    }

    // increment process counter
    dispatch_semaphore_wait(node->process_semaphore,
        DISPATCH_TIME_NOW);

    // invoke new procces
    dispatch_async(dispatch_queue, ^ {
            // call process kernel
            function(process);

            // cleanup process
            actor_process_release(process);

            // decrement process counter
            dispatch_semaphore_signal(node->process_semaphore);
        });

    return process->pid;
}

// message sendig
actor_message_t actor_message_send(actor_process_t process,
    actor_process_id_t dest_id, actor_message_t message) {
    // check for valid process
    if (process == NULL) {
        return NULL;
    }

    // check for correct dest_id
    if (dest_id >= process->node->message_queue_count) {
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
