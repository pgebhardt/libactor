#include "actor.h"

// spawn process
actor_error_t actor_process_spawn(actor_node_t node, actor_process_id_t* pid,
    actor_process_function_t function) {
    // check for valid node
    if (node == NULL) {
        return ACTOR_ERROR_INVALUE;
    }

    // error
    actor_error_t error = ACTOR_SUCCESS;

    // init pid to invalid
    if (pid != NULL) {
        *pid = ACTOR_INVALID_ID;
    }

    // create process
    actor_process_t process = NULL;
    if (actor_process_create(node, &process) != ACTOR_SUCCESS) {
        return ACTOR_ERROR_MEMORY;
    }

    // get dispatch queue
    dispatch_queue_t dispatch_queue = dispatch_get_global_queue(
        DISPATCH_QUEUE_PRIORITY_HIGH, 0);

    // check for success
    if (dispatch_queue == NULL) {
        // cleanup
        actor_process_release(process);

        return ACTOR_ERROR_DISPATCH;
    }

    // invoke new procces
    dispatch_async(dispatch_queue, ^ {
            // call process kernel
            actor_error_t result = function(process);

            // send error to supervisor
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
actor_error_t actor_message_send(actor_process_t process, actor_node_id_t node_id,
    actor_process_id_t dest_id, actor_message_data_t const data,
    actor_size_t size) {
    // check input
    if ((process == NULL) || (data == NULL)) {
        return ACTOR_ERROR_INVALUE;
    }

    // error
    actor_error_t error = ACTOR_SUCCESS;

    // check ids
    if ((dest_id < 0) || (node_id < 0)) {
        return ACTOR_ERROR_INVALUE;
    }

    // create message
    actor_message_t message = NULL;
    if (actor_message_create(&message, data, size) != ACTOR_SUCCESS) {
        return ACTOR_ERROR_MEMORY;
    }

    // set message destination
    message->destination = dest_id;

    // destination message queue
    actor_message_queue_t queue = NULL;

    // check node if
    if (node_id == process->node->nid) {
        // get message queue
        error = actor_node_get_message_queue(process->node, &queue, dest_id);

        // check success
        if (error != ACTOR_SUCCESS) {
            // release message
            actor_message_release(message);

            return error;
        }
    }
    else {
        // get remote node message queue
        error = actor_node_get_message_queue(process->node, &queue,
            process->node->remote_nodes[node_id]);

        // check success
        if (error != ACTOR_SUCCESS) {
            // release message
            actor_message_release(message);

            return error;
        }
    }

    // enqueue message
    return actor_message_queue_put(queue, message);
}

// message receive
actor_error_t actor_message_receive(actor_process_t process, actor_message_t* message,
    actor_time_t timeout) {
    // check for correct input
    if ((process == NULL) || (timeout < 0.0) || (message == NULL)) {
        return ACTOR_ERROR_INVALUE;
    }

    // error
    actor_error_t error = ACTOR_SUCCESS;

    // get message
    error = actor_message_queue_get(process->message_queue, message, timeout);

    // check success
    if (error != ACTOR_SUCCESS) {
        return error;
    }

    return ACTOR_SUCCESS;
}
