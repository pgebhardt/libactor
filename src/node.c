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
    actor_message_queue_t* queue, actor_process_id_t pid) {
    // check for valid input
    if ((node == NULL) || (queue == NULL)) {
        return ACTOR_ERROR_INVALUE;
    }

    // check for correct pid
    if ((pid >= node->message_queue_count) || (pid < 0)) {
        return ACTOR_ERROR_INVALUE;
    }

    // set queue pointer
    *queue = node->message_queues[pid];

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
