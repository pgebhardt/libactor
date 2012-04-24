#include "actor.h"

// spawn process
actor_error_t actor_process_spawn(actor_node_t node, actor_process_id_t* pid,
    actor_process_function_t function) {
    // check for valid node
    if (node == NULL) {
        return ACTOR_FAILURE;
    }

    // init pid to invalid
    if (pid != NULL) {
        *pid = ACTOR_INVALID_ID;
    }

    // create process
    actor_process_t process = NULL;
    if (actor_process_create(node, &process) != ACTOR_SUCCESS) {
        return ACTOR_FAILURE;
    }

    // get dispatch queue
    dispatch_queue_t dispatch_queue = dispatch_get_global_queue(
        DISPATCH_QUEUE_PRIORITY_HIGH, 0);

    // check for success
    if (dispatch_queue == NULL) {
        // cleanup
        actor_process_release(process);

        return ACTOR_FAILURE;
    }

    // invoke new procces
    dispatch_async(dispatch_queue, ^ {
            // call process kernel
            actor_error_t result = function(process);

            // on error send message to supervisor
            if (result != ACTOR_SUCCESS) {
                // create error message
                actor_process_error_message_struct error_message;
                error_message.nid = node->nid;
                error_message.pid = process->pid;
                error_message.error = result;

                // send message
                actor_message_send(process, process->supervisor_nid,
                    process->supervisor_pid, &error_message,
                    sizeof(actor_process_error_message_struct));
            }

            // cleanup process
            actor_process_release(process);
        });

    // set pid
    if (pid != NULL) {
        *pid = process->pid;
    }

    return ACTOR_SUCCESS;
}

// message sendig
actor_message_t actor_message_send(actor_process_t process, actor_node_id_t node_id,
    actor_process_id_t dest_id, actor_message_data_t const data,
    actor_size_t size) {
    // check input
    if ((process == NULL) || (data == NULL)) {
        return NULL;
    }

    // check ids
    if ((dest_id < 0) || (node_id < 0)) {
        return NULL;
    }

    // create message
    actor_message_t message = actor_message_create(data, size);

    // check success
    if (message == NULL) {
        return NULL;
    }

    // set message destination
    message->destination = dest_id;

    // destination message queue
    actor_message_queue_t queue = NULL;

    // check node if
    if (node_id == process->node->nid) {
        // get message queue
        queue = actor_node_message_queue_get(process->node, dest_id);

        // check success
        if (queue == NULL) {
            // release message
            actor_message_release(message);

            return NULL;
        }
    }
    else {
        // get remote node message queue
        queue = actor_node_message_queue_get(process->node,
            process->node->remote_nodes[node_id]);

        // check success
        if (queue == NULL) {
            // release message
            actor_message_release(message);

            return NULL;
        }
    }

    // enqueue message
    return actor_message_queue_put(queue, message);
}

// message receive
actor_message_t actor_message_receive(actor_process_t process,
    actor_time_t timeout) {
    // check for correct input
    if ((process == NULL) || (timeout < 0.0)) {
        return NULL;
    }

    // get message
    actor_message_t message = actor_message_queue_get(process->message_queue, timeout);

    // check success
    if (message == NULL) {
        return NULL;
    }

    return message;
}
