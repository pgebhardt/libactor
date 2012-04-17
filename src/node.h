#ifndef NODE_H
#define NODE_H

#include "message.h"

// node id
typedef unsigned int node_id;

// process size
typedef unsigned int node_process_size;

// message queue usage
typedef bool node_message_queue_usage;

// node struct
typedef struct {
    node_id nid;
    dispatch_queue_t serial_queue;
    dispatch_queue_t concurrent_queue;
    message_queue* process_message_queues;
    node_message_queue_usage* message_queue_usage;
    node_process_size process_size;
    node_process_size process_pos;

} node_node;

// create node
node_node* node_create(node_id id, node_process_size size);

// get free message queue
message_queue* node_message_queue_get_free(node_node* node, node_process_size* id);

// get message queue for id
message_queue* node_message_queue_get(node_node* node, node_process_size pid);

// release message queue
void node_message_queue_release(node_node* node, node_process_size pid);

// cleanup
void node_cleanup(node_node* node);

#endif
