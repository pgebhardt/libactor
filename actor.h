#ifndef ACTOR_H
#define ACTOR_H

// standard includes
#include <stdlib.h>
#include <dispatch/dispatch.h>

// standard type definitions
#include "common.h"

// error definitions
#include "error.h"

// actor includes
#include "message.h"
#include "node.h"
#include "process.h"
#include "distributer.h"

// spawn new process
actor_error_t actor_process_spawn(actor_node_t node, actor_process_id_t* pid,
    actor_process_function_t function);

// message sending
actor_error_t actor_message_send(actor_process_t process,
    actor_node_id_t destination_nid, actor_process_id_t destination_pid,
    actor_message_data_t const data, actor_size_t size);

// message receive
actor_error_t actor_message_receive(actor_process_t process, actor_message_t* message,
    actor_time_t timeout);

#endif
