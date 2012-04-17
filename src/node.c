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

    // set attributes
    node->nid = id;
    node->serial_queue = serial_queue;
    node->concurrent_queue = concurrent_queue;

    // init 
    node->process_size = size;
    node->process_message_queues = malloc(sizeof(message_queue) * size);

    for (node_process_size i = 0; i < size; i++) {
        message_queue_init(&(node->process_message_queues[i]), node->serial_queue);
    }

    node->process_count = 0;

    return node;
}

// get free message queue
message_queue* node_message_queue_get(node_node* node, unsigned int* id) {
    // check for size
    if (node->process_count == node->process_size) {
        return NULL;
    }

    // set id
    *id = node->process_count;

    // increment count
    node->process_count++;

    return &(node->process_message_queues[node->process_count - 1]);
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
