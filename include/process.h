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

#ifndef ACTOR_PROCESS_H
#define ACTOR_PROCESS_H

// Process struct
typedef struct {
    actor_process_id_t nid;
    actor_node_id_t pid;
    actor_node_t node;
    actor_node_id_t supervisor_nid;
    actor_process_id_t supervisor_pid;
    actor_message_queue_t message_queue;
    dispatch_semaphore_t sleep_semaphore;
} actor_process_s;
typedef actor_process_s* actor_process_t;

// process error message
typedef struct {
    actor_node_id_t nid;
    actor_process_id_t pid;
    actor_error_t error;
} actor_process_error_message_s;
typedef actor_process_error_message_s* actor_process_error_message_t;

// Process block signature
typedef actor_error_t (^actor_process_function_t)(actor_process_t self);

// create process
actor_error_t actor_process_create(actor_node_t node, actor_process_t* processPointer);

// release process
actor_error_t actor_process_release(actor_process_t* processPointer);

// message receive
actor_error_t actor_process_receive_message(actor_process_t process, actor_message_t* message,
    actor_time_t timeout);

// sleep
actor_error_t actor_process_sleep(actor_process_t process, actor_time_t time);

// link
actor_error_t actor_process_link(actor_process_t process,
    actor_node_id_t supervisor_nid, actor_process_id_t supervisor_pid);

// unlink
actor_error_t actor_process_unlink(actor_process_t process);

#endif
