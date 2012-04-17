#include <dispatch/dispatch.h>

#ifndef NODE_H
#define NODE_H

// node id
typedef unsigned int node_id;

// node struct
typedef struct {
    node_id nid;
    dispatch_queue_t serial_queue;
    dispatch_queue_t concurrent_queue;
} node_node;

// create node
node_node* node_create(node_id id);

// cleanup
void node_cleanup(node_node* node);

#endif
