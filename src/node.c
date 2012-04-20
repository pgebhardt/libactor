#include <stdlib.h>
#include <dispatch/dispatch.h>
#include "node.h"

// create node
actor_node_t actor_node_create(actor_node_id_t id, actor_node_process_size_t size) {
    // create node
    actor_node_t node = malloc(sizeof(actor_node_struct));

    // check for success
    if (node == NULL) {
        return NULL;
    }

    // set attributes
    node->nid = id;

    // init
    node->process_size = size;
    node->process_message_queues = malloc(sizeof(actor_message_queue_struct) * size);

    // check for success
    if (node->process_message_queues == NULL) {
        return NULL;
    }

    // message queue usage
    node->message_queue_usage = malloc(sizeof(actor_node_message_queue_usage_t) * size);

    // check success
    if (node->message_queue_usage == NULL) {
        return NULL;
    }

    for (actor_node_process_size_t i = 0; i < size; i++) {
        actor_message_queue_init(&(node->process_message_queues[i]));
        node->message_queue_usage[i] = false;
    }

    node->process_pos = 0;

    return node;
}

// start process
actor_process_t actor_node_start_process(actor_node_t node,
    actor_process_function_t function, bool blocking) {
    // check for valid node
    if (node == NULL) {
        return NULL;
    }

    // get free message queue
    actor_process_id_t id = 0;
    actor_message_queue_t queue = actor_node_message_queue_get_free(node, &id);

    // check for valid queue
    if (queue == NULL) {
        return NULL;
    }

    // create process struct
    __block actor_process_t process = malloc(sizeof(actor_process_struct));

    // check for success
    if (process == NULL) {
        return NULL;
    }

    // create dispatch queue
    process->dispatch_queue = dispatch_get_global_queue(
        DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);

    // check for success
    if (process->dispatch_queue == NULL) {
        return NULL;
    }

    // save attributes
    process->message_queue = queue;
    process->pid = id;
    process->node = node;

    // call process function
    if (blocking == true) {
        dispatch_sync(process->dispatch_queue, ^ {
                // call process kernel
                function(process);
            });

        // cleanup process
        actor_process_cleanup(process);

        // set process pointer to NULL
        process = NULL;

    }
    else {
        dispatch_async(process->dispatch_queue, ^ {
                // call process kernel
                function(process);

                // cleanup process
                actor_process_cleanup(process);
            });
    }

    return process;
}

// spawn new process
actor_process_id_t actor_process_spawn(actor_node_t node,
    actor_process_function_t function) {
    // check for valid node
    if (node == NULL) {
        return -1;
    }

    // start non blocking process
    actor_process_t process = actor_node_start_process(node, function, false);

    // check for success
    if (process == NULL) {
        return -1;
    }

    return process->pid;
}

// start main process
void actor_main_process(actor_node_t node, actor_process_function_t function) {
    // check for valid node
    if (node == NULL) {
        return;
    }

    // start blocking process
    actor_node_start_process(node, function, true);
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

    // cleanup message queues
    for (actor_node_process_size_t i = 0; i < node->process_size; i++) {
        actor_message_queue_cleanup(&(node->process_message_queues[i]));
    }

    // free message queues
    free(node->process_message_queues);

    // free queue usage
    free(node->message_queue_usage);
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
