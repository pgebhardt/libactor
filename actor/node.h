#ifndef NODE_H
#define NODE_H

// node id
typedef unsigned int actor_node_id_t;

// node struct
typedef struct {
    actor_node_id_t nid;
    actor_message_queue_t* message_queues;
    actor_size_t message_queue_count;
    actor_size_t message_queue_pos;
    dispatch_semaphore_t process_semaphore;
} actor_node_struct;
typedef actor_node_struct* actor_node_t;

#include "process.h"

// create node
actor_node_t actor_node_create(actor_node_id_t id, actor_size_t size);

// cleanup
void actor_node_release(actor_node_t node);

// get free message queue
actor_message_queue_t actor_node_message_queue_get_free(
    actor_node_t node, actor_process_id_t* id);

// get message queue for id
actor_message_queue_t actor_node_message_queue_get(actor_node_t node,
    actor_process_id_t pid);

// release message queue
void actor_node_message_queue_release(actor_node_t node, actor_process_id_t pid);

#endif
