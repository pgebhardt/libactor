#ifndef ACTOR_H
#define ACTOR_H

// standard includes
#include <stdlib.h>
#include <dispatch/dispatch.h>

// error
// error type
typedef unsigned int actor_error_t;

// standart errors
#define ACTOR_SUCCESS (0)
#define ACTOR_FAILURE (1)

//standard types
// size
typedef unsigned int actor_size_t;

// time
typedef double actor_time_t;

// node id
typedef int actor_node_id_t;

// process id
typedef int actor_process_id_t;

#define ACTOR_INVALID_ID (-1)

// actor includes
#include "message.h"
#include "node.h"
#include "process.h"
#include "distributer.h"

// spawn new process
actor_error_t actor_process_spawn(actor_node_t node, actor_process_id_t* pid,
    actor_process_function_t function);

// message sending
actor_error_t actor_message_send(actor_process_t process, actor_node_id_t node_id,
    actor_process_id_t dest_id, actor_message_data_t const data,
    actor_size_t size);

// message receive
actor_error_t actor_message_receive(actor_process_t process, actor_message_t* message,
    actor_time_t timeout);

#endif
