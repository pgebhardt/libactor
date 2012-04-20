#ifndef NODE_H
#define NODE_H

#include "message.h"

// node id
typedef unsigned int actor_node_id_t;

// process size
typedef unsigned int actor_node_process_size_t;

// message queue usage
typedef bool actor_node_message_queue_usage_t;

// node struct
typedef struct {
    actor_node_id_t nid;
    actor_message_queue_t process_message_queues;
    actor_node_message_queue_usage_t* message_queue_usage;
    actor_node_process_size_t process_size;
    actor_node_process_size_t process_pos;

} actor_node_struct;
typedef actor_node_struct* actor_node_t;

// create node
actor_node_t actor_node_create(actor_node_id_t id, actor_node_process_size_t size);

// include process here, to make shure node struct is defined
#include "process.h"

// start process
actor_process_t actor_node_start_process(actor_node_t node,
    actor_process_function_t function, bool blocking);

// spawn new process
actor_process_id_t actor_process_spawn(actor_node_t node,
    actor_process_function_t function);

// start main process
void actor_main_process(actor_node_t node, actor_process_function_t function);

// get free message queue
actor_message_queue_t actor_node_message_queue_get_free(
    actor_node_t node, actor_process_id_t* id);

// get message queue for id
actor_message_queue_t actor_node_message_queue_get(actor_node_t node,
    actor_process_id_t pid);

// release message queue
void actor_node_message_queue_release(actor_node_t node, actor_process_id_t pid);

// cleanup
void actor_node_cleanup(actor_node_t node);
void actor_node_release(actor_node_t node);

#endif
