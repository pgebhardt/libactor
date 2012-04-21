#ifndef ACTOR_H
#define ACTOR_H

// standard includes
#include <stdlib.h>
#include <dispatch/dispatch.h>

//standard types
// size
typedef unsigned int actor_size_t;

// actor includes
#include "message.h"
#include "node.h"
#include "process.h"

// spawn new process
actor_process_id_t actor_process_spawn(actor_node_t node,
    actor_process_function_t function);

// message sending
actor_message_t actor_message_send(actor_process_t process,
    actor_process_id_t dest_id, actor_message_t message);

// message receive
actor_message_t actor_message_receive(actor_process_t process,
    double timeout);

#endif
