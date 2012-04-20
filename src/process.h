#ifndef PROCESS_H
#define PROCESS_H

#include <dispatch/dispatch.h>
#include "message.h"
#include "node.h"

// Process Identifier
typedef int actor_process_id_t;

// Process struct
typedef struct {
    actor_process_id_t pid;
    actor_message_queue_t message_queue;
    dispatch_queue_t dispatch_queue;
    actor_node_t node;
} actor_process_struct;
typedef actor_process_struct* actor_process_t;

// Process block signature
typedef void (^actor_process_function_t)(actor_process_t const self);

// message sending
actor_message_t actor_message_send(actor_process_t process,
    actor_process_id_t dest_id, actor_message_t message);

// message receive
actor_message_t actor_message_receive(actor_process_t process,
    double timeout);

// process cleanup
void actor_process_cleanup(actor_process_t process);
void actor_process_release(actor_process_t process);

#endif
