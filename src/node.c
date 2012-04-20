#include <stdlib.h>
#include <dispatch/dispatch.h>
#include "node.h"

// create node
actor_node_t actor_node_create(actor_node_id_t id, actor_node_process_size_t size) {
    // create process semaphore
    dispatch_semaphore_t process_semaphore = dispatch_semaphore_create(0);

    // check success
    if (process_semaphore == NULL) {
        return NULL;
    }

    // create process message queues
    actor_message_queue_t process_message_queues = malloc(
        sizeof(actor_message_queue_struct) * size);

    // check success
    if (process_message_queues == NULL) {
        // cleanup
        dispatch_release(process_semaphore);

        return NULL;
    }

    // create message queue usage array
    actor_node_message_queue_usage_t* message_queue_usage = malloc(
        sizeof(actor_node_message_queue_usage_t) * size);

    // check success
    if (message_queue_usage == NULL) {
        // cleanup
        dispatch_release(process_semaphore);
        free(process_message_queues);

        return NULL;
    }

    // create node
    actor_node_t node = malloc(sizeof(actor_node_struct));

    // check success
    if (node == NULL) {
        // cleanup
        dispatch_release(process_semaphore);
        free(process_message_queues);
        free(message_queue_usage);

        return NULL;
    }

    // init
    node->process_semaphore = process_semaphore;
    node->process_message_queues = process_message_queues;
    node->message_queue_usage = message_queue_usage;
    node->nid = id;
    node->process_size = size;
    node->process_pos = 0;

    // init message queues
    for (actor_node_process_size_t i = 0; i < size; i++) {
        actor_message_queue_init(&(node->process_message_queues[i]));
        node->message_queue_usage[i] = false;
    }

    return node;
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
    actor_process_id_t id = node->process_pos;

    // check for correct id
    if ((id >= node->process_size) || (node->message_queue_usage[id] == true)) {
        // look for first free queue
        for (actor_node_process_size_t i = 0; i < node->process_size; i++) {
            // check for used queue
            if (node->message_queue_usage[i] == false) {
                // set new id
                id = i;

                break;
            }
        }

        // check new id
        if (id >= node->process_size) {
            return NULL;
        }

        // set new id
        *pid = id;
        node->process_pos = id + 1;
    }
    else {
        // set id
        *pid = node->process_pos;

        // increment count
        node->process_pos++;
    }

    // mark queue as used
    node->message_queue_usage[*pid] = true;

    return &(node->process_message_queues[*pid]);
}

// get message queue for id
actor_message_queue_t actor_node_message_queue_get(actor_node_t node,
    actor_process_id_t pid) {
    // check for valid node
    if (node == NULL) {
        return NULL;
    }

    // check for correct pid
    if (pid >= node->process_size) {
        return NULL;
    }

    return &node->process_message_queues[pid];
}

// release message queue
void actor_node_message_queue_release(actor_node_t node, actor_process_id_t pid) {
    // check for correct pid
    if (pid >= node->process_size) {
        return;
    }

    // set queue as unused
    node->message_queue_usage[pid] = false;
}

// cleanup
void actor_node_cleanup(actor_node_t node) {
    // check for valid node
    if (node == NULL) {
        return;
    }

    // wait for processes to complete
    dispatch_semaphore_wait(node->process_semaphore, DISPATCH_TIME_FOREVER);

    // cleanup message queues
    for (actor_node_process_size_t i = 0; i < node->process_size; i++) {
        actor_message_queue_cleanup(&(node->process_message_queues[i]));
    }

    // free message queues
    free(node->process_message_queues);

    // free queue usage
    free(node->message_queue_usage);

    // release process semaphore
    dispatch_release(node->process_semaphore);
}

void actor_node_release(actor_node_t node) {
    // check for valid node
    if (node == NULL) {
        return;
    }

    // cleanup node
    actor_node_cleanup(node);

    // free memory
    free(node);
}
