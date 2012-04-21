#include <stdlib.h>
#include <dispatch/dispatch.h>
#include "node.h"

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
    node->message_queue_count = size;
    node->message_queue_pos = 0;
    node->process_semaphore = NULL;

    // create message queues
    node->message_queues = malloc(sizeof(actor_message_queue_t) * size);

    // check success
    if (node->message_queues == NULL) {
        // release node
        actor_node_release(node);

        return NULL;
    }

    // create process semaphore
    node->process_semaphore = dispatch_semaphore_create(0);

    // check success
    if (node->process_semaphore == NULL) {
        return NULL;
    }

    return node;
}

void actor_node_release(actor_node_t node) {
    // check for valid node
    if (node == NULL) {
        return;
    }

    // release message queues
    if (node->message_queues != NULL) {
        for (actor_size_t i = 0; i < node->message_queue_count; i++) {
            actor_message_queue_release(node->message_queues[i]);
        }

        free(node->message_queues);
    }

    // release process semaphore
    if (node->process_semaphore != NULL) {
        dispatch_release(node->process_semaphore);
    }

    // free memory
    free(node);
}
// start process
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

// get free message queue
actor_message_queue_t actor_node_message_queue_get_free(actor_node_t node,
    actor_process_id_t* pid) {
    // check for correct input
    if ((node == NULL) || (pid == NULL)) {
        return NULL;
    }

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
void actor_node_message_queue_release(actor_node_t node, actor_process_id_t pid) {
    // check for correct pid
    if (pid >= node->message_queue_count) {
        return;
    }

    // release message queue
    actor_message_queue_release(node->message_queues[pid]);

    // set queue pointer to NULL
    node->message_queues[pid] = NULL;
}

