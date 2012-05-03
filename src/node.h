#ifndef NODE_H
#define NODE_H

// maximum remote nodes count
#define ACTOR_NODE_MAX_REMOTE_NODES (1024)

// node struct
typedef struct {
    actor_node_id_t id;
    actor_message_queue_t* message_queues;
    actor_process_id_t* remote_nodes;
    actor_size_t message_queue_count;
    actor_size_t message_queue_pos;
    dispatch_semaphore_t process_semaphore;
    dispatch_semaphore_t message_queue_create_semaphore;
    actor_size_t process_count;
} actor_node_s;
typedef actor_node_s* actor_node_t;

#include "process.h"

// create node
actor_error_t actor_node_create(actor_node_t* node, actor_node_id_t id,
    actor_size_t size);

// cleanup
actor_error_t actor_node_release(actor_node_t node);

// get free message queue
actor_error_t actor_node_get_free_message_queue(actor_node_t node,
    actor_message_queue_t* queue, actor_process_id_t* id);

// get message queue for id
actor_error_t actor_node_get_message_queue(actor_node_t node,
    actor_message_queue_t* queue, actor_process_id_t id);

// release message queue
actor_error_t actor_node_message_queue_release(actor_node_t node,
    actor_process_id_t pid);

// connect to remote node
actor_error_t actor_node_connect(actor_node_t node, actor_node_id_t* nid,
    char* const host_name, unsigned int host_port, const char* key);

// listen for incomming connection
actor_error_t actor_node_listen(actor_node_t node, actor_node_id_t* nid,
    unsigned int port, const char* key);

#endif
