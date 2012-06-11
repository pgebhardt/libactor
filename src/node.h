// libactor
//
// Implementation of an erlang style actor model using libdispatch
// Copyright (C) 2012  Patrik Gebhardt
// Contact: patrik.gebhardt@rub.de
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef ACTOR_NODE_H
#define ACTOR_NODE_H

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
actor_error_t actor_node_create(actor_node_t* nodePointer, actor_node_id_t id,
    actor_size_t size);

// cleanup
actor_error_t actor_node_release(actor_node_t* nodePointer);

// spawn new process
actor_error_t actor_node_spawn_process(actor_node_t node, actor_process_id_t* pid,
    actor_process_function_t function);

// message sending
actor_error_t actor_node_send_message(actor_node_t node,
    actor_node_id_t destination_nid, actor_process_id_t destination_pid,
    actor_data_type_t type, actor_message_data_t const data, actor_size_t size);

// get free message queue
actor_error_t actor_node_get_free_message_queue(actor_node_t node,
    actor_message_queue_t* queue, actor_process_id_t* id);

// get message queue for id
actor_error_t actor_node_get_message_queue(actor_node_t node,
    actor_message_queue_t** queue, actor_process_id_t id);

// release message queue
actor_error_t actor_node_message_queue_release(actor_node_t node,
    actor_process_id_t pid);

// wait for processes to complete
actor_error_t actor_node_wait_for_processes(actor_node_t node, actor_time_t timeout);

// connect to remote node
actor_error_t actor_node_connect(actor_node_t node, actor_node_id_t* nid,
    const char* host_name, unsigned int host_port, const char* key);

// listen for incomming connection
actor_error_t actor_node_listen(actor_node_t node, actor_node_id_t* nid,
    unsigned int port, const char* key);

// close connection
actor_error_t actor_node_disconnect(actor_node_t node, actor_node_id_t nid);

#endif
