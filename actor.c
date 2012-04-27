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
    error = actor_process_create(node, &process);

    // check success
    if (error != ACTOR_SUCCESS) {
        return error;
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

        // create error message
        actor_process_error_message_s error_message;
        error_message.nid = process->nid;
        error_message.pid = process->pid;
        error_message.error = result;

        // send message
        actor_message_send(process, process->supervisor_nid,
            process->supervisor_pid, ACTOR_TYPE_ERROR_MESSAGE, &error_message,
            sizeof(actor_process_error_message_s));

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
actor_error_t actor_message_send(actor_process_t process,
    actor_node_id_t destination_nid, actor_process_id_t destination_pid,
    actor_data_type_t type, actor_message_data_t const data, actor_size_t size) {
    // check input
    if ((process == NULL) || (data == NULL) || (type < 0)) {
        return ACTOR_ERROR_INVALUE;
    }

    // error
    actor_error_t error = ACTOR_SUCCESS;

    // check ids
    if ((destination_nid < 0) || (destination_pid < 0)) {
        return ACTOR_ERROR_INVALUE;
    }

    // create message
    actor_message_t message = NULL;
    if (actor_message_create(&message, type, data, size) != ACTOR_SUCCESS) {
        return ACTOR_ERROR_MEMORY;
    }

    // set message destination
    message->destination_nid = destination_nid;
    message->destination_pid = destination_pid;

    // destination message queue
    actor_message_queue_t queue = NULL;

    // check node id
    if (destination_nid == process->nid) {
        // get message queue
        error = actor_node_get_message_queue(process->node, &queue, destination_pid);

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
            process->node->remote_nodes[destination_nid]);

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
