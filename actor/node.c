#include "actor.h"

// create node
actor_node_t actor_node_create(actor_node_id_t id, actor_size_t size) {
    // create node
    actor_node_t node = malloc(sizeof(actor_node_struct));

    // check success
    if (node == NULL) {
        return NULL;
    }

    // init struct
    node->nid = id;
    node->message_queues = NULL;
    node->remote_nodes = NULL;
    node->message_queue_count = size;
    node->message_queue_pos = 0;
    node->process_semaphore = NULL;
    node->message_queue_create_semaphore = NULL;
    node->process_count = 0;

    // create message queues
    node->message_queues = malloc(sizeof(actor_message_queue_t) * size);

    // check success
    if (node->message_queues == NULL) {
        // release node
        actor_node_release(node);

        return NULL;
    }

    // create remote node array
    node->remote_nodes = malloc(sizeof(int) * 1024);

    // check success
    if (node->remote_nodes == NULL) {
        // release node
        actor_node_release(node);

        return NULL;
    }

    // init array
    for (actor_size_t i = 0; i < 1024; i++) {
        node->remote_nodes[i] = -1;
    }

    // create process semaphore
    node->process_semaphore = dispatch_semaphore_create(0);

    // check success
    if (node->process_semaphore == NULL) {
        // release node
        actor_node_release(node);

        return NULL;
    }

    // create message queue create semaphore
    node->message_queue_create_semaphore = dispatch_semaphore_create(1);

    // check success
    if (node->message_queue_create_semaphore == NULL) {
        // release node
        actor_node_release(node);

        return NULL;
    }

    return node;
}

actor_error_t actor_node_release(actor_node_t node) {
    // check for valid node
    if (node == NULL) {
        return ACTOR_FAILURE;
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
actor_message_queue_t actor_node_message_queue_get_free(actor_node_t node,
    actor_process_id_t* pid) {
    // check for correct input
    if ((node == NULL) || (pid == NULL)) {
        return NULL;
    }

    // get message queue create access
    dispatch_semaphore_wait(node->message_queue_create_semaphore, DISPATCH_TIME_FOREVER);

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
        if (id >= node->message_queue_pos) {
            return NULL;
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
    node->message_queues[*pid] = actor_message_queue_create();

    // check success
    if (node->message_queues[*pid] == NULL) {
        return NULL;
    }

    // increment process counter
    node->process_count++;
    dispatch_semaphore_wait(node->process_semaphore, DISPATCH_TIME_NOW);

    // release message queue create access
    dispatch_semaphore_signal(node->message_queue_create_semaphore);

    return node->message_queues[*pid];
}

// get message queue for id
actor_message_queue_t actor_node_message_queue_get(actor_node_t node,
    actor_process_id_t pid) {
    // check for valid node
    if (node == NULL) {
        return NULL;
    }

    // check for correct pid
    if (pid >= node->message_queue_count) {
        return NULL;
    }

    return node->message_queues[pid];
}

// release message queue
actor_error_t actor_node_message_queue_release(actor_node_t node, actor_process_id_t pid) {
    // check for correct pid
    if (pid >= node->message_queue_count) {
        return ACTOR_FAILURE;
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
actor_node_id_t actor_node_connect(actor_node_t node,
    char* const host_name, unsigned int host_port) {
    // check for valid node
    if (node == NULL) {
        return -1;
    }

    // connect to remote
    return actor_distributer_connect_to_node(node, host_name, host_port);
}

// listen for incomming connection
actor_node_id_t actor_node_listen(actor_node_t node, unsigned int port) {
    // check for valid node
    if (node == NULL) {
        return -1;
    }

    // start listening
    return actor_distributer_listen(node, port);
}
