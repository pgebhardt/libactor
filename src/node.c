#include <stdlib.h>
#include "node.h"

// create node
node_node* node_create(node_id id) {
    // create serial queue for message passing
    dispatch_queue_t serial_queue = dispatch_queue_create("de.rub.est.actor", NULL);

    // get concurrent queue
    dispatch_queue_t concurrent_queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);

    // create node
    node_node* node = malloc(sizeof(node_node));

    // set attributes
    node->nid = id;
    node->serial_queue = serial_queue;
    node->concurrent_queue = concurrent_queue;

    return node;
}

// cleanup
void node_cleanup(node_node* node) {
    // release serial queue
    dispatch_release(node->serial_queue);

    // free memory
    free(node);
}
