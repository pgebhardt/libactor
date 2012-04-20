#include <stdlib.h>
#include <dispatch/dispatch.h>
#include "node.h"

// create node
node_node* node_create(node_id id, node_process_size size) {
    // create node
    node_node* node = malloc(sizeof(node_node));

    // check for success
    if (node == NULL) {
        return NULL;
    }

    // set attributes
    node->nid = id;

    // init
    node->process_size = size;
    node->process_message_queues = malloc(sizeof(message_queue) * size);

    // check for success
    if (node->process_message_queues == NULL) {
        return NULL;
    }

    // message queue usage
    node->message_queue_usage = malloc(sizeof(node_message_queue_usage) * size);

    // check success
    if (node->message_queue_usage == NULL) {
        return NULL;
    }

    for (node_process_size i = 0; i < size; i++) {
        message_queue_init(&(node->process_message_queues[i]));
        node->message_queue_usage[i] = false;
    }

    node->process_pos = 0;

    return node;
}

// start process
process_process* node_start_process(node_node* node, process_process_function function,
    bool blocking) {
    // check for valid node
    if (node == NULL) {
        return NULL;
    }

    // get free message queue
    process_id id = 0;
    message_queue* queue = node_message_queue_get_free(node, &id);

    // check for valid queue
    if (queue == NULL) {
        return NULL;
    }

    // create process struct
    __block process_process* process = malloc(sizeof(process_process));

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
        process_cleanup(process);

        // set process pointer to NULL
        process = NULL;

    }
    else {
        dispatch_async(process->dispatch_queue, ^ {
                // call process kernel
                function(process);

                // cleanup process
                process_cleanup(process);
            });
    }

    return process;
}

// spawn new process
process_id node_process_spawn(node_node* node, process_process_function function) {
    // check for valid node
    if (node == NULL) {
        return -1;
    }

    // start non blocking process
    process_process* process = node_start_process(node, function, false);

    return process->pid;
}

// start main process
void node_main_process(node_node* node, process_process_function function) {
    // check for valid node
    if (node == NULL) {
        return;
    }

    // start blocking process
    node_start_process(node, function, true);
}

// get free message queue
message_queue* node_message_queue_get_free(node_node* node, process_id* pid) {
    // check for correct input
    if ((node == NULL) || (pid == NULL)) {
        return NULL;
    }

    // get possible id
    process_id id = node->process_pos;

    // check for correct id
    if ((id >= node->process_size) || (node->message_queue_usage[id] == true)) {
        // look for first free queue
        for (node_process_size i = 0; i < node->process_size; i++) {
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
message_queue* node_message_queue_get(node_node* node, process_id pid) {
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
void node_message_queue_release(node_node* node, process_id pid) {
    // check for correct pid
    if (pid >= node->process_size) {
        return;
    }

    // set queue as unused
    node->message_queue_usage[pid] = false;
}

// cleanup
void node_cleanup(node_node* node) {
    // check for valid node
    if (node == NULL) {
        return;
    }

    // cleanup message queues
    for (node_process_size i = 0; i < node->process_size; i++) {
        message_queue_cleanup(&(node->process_message_queues[i]));
    }

    // free message queues
    free(node->process_message_queues);

    // free queue usage
    free(node->message_queue_usage);
}

void node_release(node_node* node) {
    // check for valid node
    if (node == NULL) {
        return;
    }

    // cleanup node
    node_cleanup(node);

    // free memory
    free(node);
}
