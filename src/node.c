#include <stdlib.h>
#include <dispatch/dispatch.h>
#include "node.h"

// create node
node_node* node_create(node_id id, node_process_size size) {
    // create serial queue for message passing
    dispatch_queue_t serial_queue = dispatch_queue_create("de.rub.est.actor", NULL);

    // get concurrent queue
    dispatch_queue_t concurrent_queue = dispatch_get_global_queue(
        DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);

    // create node
    node_node* node = malloc(sizeof(node_node));

    // check for success
    if (node == NULL) {
        return NULL;
    }

    // set attributes
    node->nid = id;
    node->serial_queue = serial_queue;
    node->concurrent_queue = concurrent_queue;

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
        message_queue_init(&(node->process_message_queues[i]), node->serial_queue);
        node->message_queue_usage[i] = false;
    }

    node->process_pos = 0;

    return node;
}

// get free message queue
message_queue* node_message_queue_get_free(node_node* node, node_process_size* pid) {
    // check for correct input
    if ((node == NULL) || (pid == NULL)) {
        return NULL;
    }

    // get possible id
    node_process_size id = node->process_pos;

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
message_queue* node_message_queue_get(node_node* node, node_process_size pid) {
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
void node_message_queue_release(node_node* node, node_process_size pid) {
    // check for correct pid
    if (pid >= node->process_size) {
        return;
    }

    // set queue as unused
    node->message_queue_usage[pid] = false;
}

// cleanup
void node_cleanup(node_node* node) {
    // release serial queue
    dispatch_release(node->serial_queue);

    // free message queues
    free(node->process_message_queues);

    // free memory
    free(node);
}
