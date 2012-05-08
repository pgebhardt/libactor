#include "actor.h"

// create node
actor_error_t actor_node_create(actor_node_t* node, actor_node_id_t id,
    actor_size_t size) {
    // check valid node pointer
    if (node == NULL) {
        return ACTOR_ERROR_INVALUE;
    }

    // init node pointer to NULL
    *node = NULL;

    // create node
    actor_node_t newNode = malloc(sizeof(actor_node_s));

    // check success
    if (newNode == NULL) {
        return ACTOR_ERROR_MEMORY;
    }

    // init struct
    newNode->id = id;
    newNode->message_queues = NULL;
    newNode->remote_nodes = NULL;
    newNode->message_queue_count = size;
    newNode->message_queue_pos = 0;
    newNode->process_semaphore = NULL;
    newNode->message_queue_create_semaphore = NULL;
    newNode->process_count = 0;

    // create message queues
    newNode->message_queues = malloc(sizeof(actor_message_queue_t) * size);

    // check success
    if (newNode->message_queues == NULL) {
        // release node
        actor_node_release(newNode);

        return ACTOR_ERROR_MEMORY;
    }

    // create remote node array
    newNode->remote_nodes = malloc(sizeof(int) * ACTOR_NODE_MAX_REMOTE_NODES);

    // check success
    if (newNode->remote_nodes == NULL) {
        // release node
        actor_node_release(newNode);

        return ACTOR_ERROR_MEMORY;
    }

    // init array
    for (actor_size_t i = 0; i < ACTOR_NODE_MAX_REMOTE_NODES; i++) {
        newNode->remote_nodes[i] = -1;
    }

    // create process semaphore
    newNode->process_semaphore = dispatch_semaphore_create(0);

    // check success
    if (newNode->process_semaphore == NULL) {
        // release node
        actor_node_release(newNode);

        return ACTOR_ERROR_DISPATCH;
    }

    // create message queue create semaphore
    newNode->message_queue_create_semaphore = dispatch_semaphore_create(1);

    // check success
    if (newNode->message_queue_create_semaphore == NULL) {
        // release node
        actor_node_release(newNode);

        return ACTOR_ERROR_DISPATCH;
    }

    // set node pointer
    *node = newNode;

    return ACTOR_SUCCESS;
}

actor_error_t actor_node_release(actor_node_t node) {
    // check for valid node
    if (node == NULL) {
        return ACTOR_ERROR_INVALUE;
    }

    // wait for all processes to complete
    dispatch_semaphore_wait(node->process_semaphore, DISPATCH_TIME_FOREVER);

    // release message queues
    if (node->message_queues != NULL) {
        for (actor_size_t i = 0; i < node->message_queue_count; i++) {
            actor_message_queue_release(node->message_queues[i]);
        }

        free(node->message_queues);
    }

    // release remote node array
    if (node->remote_nodes != NULL) {
        free(node->remote_nodes);
    }

    // release process semaphore
    if (node->process_semaphore != NULL) {
        dispatch_release(node->process_semaphore);
    }

    // release message queue create semaphore
    if (node->message_queue_create_semaphore != NULL) {
        dispatch_release(node->message_queue_create_semaphore);
    }

    // free memory
    free(node);

    return ACTOR_SUCCESS;
}

// spawn new process
actor_error_t actor_node_spawn_process(actor_node_t node, actor_process_id_t* pid,
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
        actor_send(process, process->supervisor_nid,
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

// message sending
actor_error_t actor_node_send_message(actor_node_t node,
    actor_node_id_t destination_nid, actor_process_id_t destination_pid,
    actor_data_type_t type, actor_message_data_t const data, actor_size_t size) {
    // check input
    if ((node == NULL) || (data == NULL) || (type < 0)) {
        return ACTOR_ERROR_INVALUE;
    }

    // error
    actor_error_t error = ACTOR_SUCCESS;

    // check destination nid
    if ((destination_nid < 0) || (destination_nid >= ACTOR_NODE_MAX_REMOTE_NODES)) {
        return ACTOR_ERROR_INVALUE;
    }

    // check destination pid
    if (destination_pid < 0) {
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
    actor_message_queue_t* queue = NULL;

    // check node id
    if (destination_nid == node->id) {
        // get message queue
        error = actor_node_get_message_queue(node, &queue, destination_pid);

        // check success
        if (error != ACTOR_SUCCESS) {
            // release message
            actor_message_release(message);

            return error;
        }
    }
    else {
        // get remote node message queue
        error = actor_node_get_message_queue(node, &queue,
            node->remote_nodes[destination_nid]);

        // check success
        if (error != ACTOR_SUCCESS) {
            // release message
            actor_message_release(message);

            return error;
        }
    }

    // enqueue message
    return actor_message_queue_put(*queue, message);
}

// get free message queue
actor_error_t actor_node_get_free_message_queue(actor_node_t node,
    actor_message_queue_t* queue, actor_process_id_t* pid) {
    // check for correct input
    if ((node == NULL) || (pid == NULL) || (queue == NULL)) {
        return ACTOR_ERROR_INVALUE;
    }

    // error
    actor_error_t error = ACTOR_SUCCESS;

    // init queue and pid pointer
    *queue = NULL;
    *pid = ACTOR_INVALID_ID;

    // get message queue create access
    dispatch_semaphore_wait(node->message_queue_create_semaphore,
        DISPATCH_TIME_FOREVER);

    // get possible id
    actor_process_id_t id = node->message_queue_pos;

    // check for correct id
    if ((id >= node->message_queue_count) ||
            (node->message_queues[id] != NULL)) {
        // look for first free queue
        for (actor_size_t i = 0; i < node->message_queue_count; i++) {
            // check for used queue
            if (node->message_queues[i] == NULL) {
                // set new id
                id = i;

                break;
            }
        }

        // check new id
        if (id == node->message_queue_pos) {
            // release message queue create access
            dispatch_semaphore_signal(node->message_queue_create_semaphore);

            return ACTOR_ERROR_TOO_MANY_PROCESSES;
        }

        // set new id
        *pid = id;
        node->message_queue_pos = id + 1;
    }
    else {
        // set id
        *pid = node->message_queue_pos;

        // increment count
        node->message_queue_pos++;
    }

    // create new message queue
    actor_message_queue_t newQueue = NULL;
    error = actor_message_queue_create(&newQueue);

    // check success
    if (error != ACTOR_SUCCESS) {
        // release message queue create access
        dispatch_semaphore_signal(node->message_queue_create_semaphore);

        return error;
    }

    // register queue
    node->message_queues[*pid] = newQueue;

    // increment process counter
    node->process_count++;
    dispatch_semaphore_wait(node->process_semaphore, DISPATCH_TIME_NOW);

    // release message queue create access
    dispatch_semaphore_signal(node->message_queue_create_semaphore);

    // set queue pointer
    *queue = newQueue;

    return ACTOR_SUCCESS;
}

// get message queue for id
actor_error_t actor_node_get_message_queue(actor_node_t node,
    actor_message_queue_t** queue, actor_process_id_t pid) {
    // check for valid input
    if ((node == NULL) || (queue == NULL)) {
        return ACTOR_ERROR_INVALUE;
    }

    // check for correct pid
    if ((pid >= node->message_queue_count) || (pid < 0)) {
        return ACTOR_ERROR_INVALUE;
    }

    // set queue pointer
    *queue = &node->message_queues[pid];

    return ACTOR_SUCCESS;
}

// release message queue
actor_error_t actor_node_message_queue_release(actor_node_t node,
    actor_process_id_t pid) {
    // check for correct pid
    if ((pid >= node->message_queue_count) || (pid < 0)) {
        return ACTOR_ERROR_INVALUE;
    }

    // release message queue
    actor_message_queue_release(node->message_queues[pid]);

    // set queue pointer to NULL
    node->message_queues[pid] = NULL;

    // decrement process counter
    node->process_count--;

    // send signal if no process left
    if (node->process_count == 0) {
        dispatch_semaphore_signal(node->process_semaphore);
    }

    return ACTOR_SUCCESS;
}

// connect to remote node
actor_error_t actor_node_connect(actor_node_t node, actor_node_id_t* nid,
    char* const host_name, unsigned int host_port, const char* key) {
    // check for valid node
    if (node == NULL) {
        return ACTOR_ERROR_INVALUE;
    }

    // connect to remote
    return actor_distributer_connect_to_node(node, nid, host_name, host_port, key);
}

// listen for incomming connection
actor_error_t actor_node_listen(actor_node_t node, actor_node_id_t* nid,
    unsigned int port, const char* key) {
    // check for valid node
    if (node == NULL) {
        return ACTOR_ERROR_INVALUE;
    }

    // start listening
    return actor_distributer_listen(node, nid, port, key);
}

// close connection
actor_error_t actor_node_disconnect(actor_node_t node, actor_node_id_t nid) {
    // check input
    if ((node == NULL) || (nid < 0) || (nid >= ACTOR_NODE_MAX_REMOTE_NODES)) {
        return ACTOR_ERROR_INVALUE;
    }

    // check connection state
    if (node->remote_nodes[nid] == ACTOR_INVALID_ID) {
        return ACTOR_ERROR_NETWORK;
    }

    // send disconnect message
    return actor_node_send_message(node, node->id, node->remote_nodes[nid],
        ACTOR_TYPE_CHAR, "STOP", 5);
}
