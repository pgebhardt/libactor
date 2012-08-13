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

#ifndef ACTOR_MESSAGE_H
#define ACTOR_MESSAGE_H

// message data
typedef void* actor_message_data_t;

// message struct
typedef struct {
    struct actor_message_s* next;
    actor_node_id_t destination_nid;
    actor_process_id_t destination_pid;
    actor_size_t size;
    actor_message_data_t data;
    actor_data_type_t type;
} actor_message_s;
typedef actor_message_s* actor_message_t;

// message queue
typedef struct {
    dispatch_semaphore_t semaphore_read_write;
    dispatch_semaphore_t semaphore_messages;
    actor_message_t first;
    actor_message_t last;
} actor_message_queue_s;
typedef actor_message_queue_s* actor_message_queue_t;

// create new message
actor_error_t actor_message_create(actor_message_t* messagePointer,
    actor_data_type_t type, actor_message_data_t const data, actor_size_t size);

// cleanup message
actor_error_t actor_message_release(actor_message_t* messagePointer);

// create new queue
actor_error_t actor_message_queue_create(actor_message_queue_t* queuePointer);

// cleanup queue
actor_error_t actor_message_queue_release(actor_message_queue_t* queuePointer);

// add new message to queue
actor_error_t actor_message_queue_put(actor_message_queue_t queue,
    actor_message_t message);

// get message from queue
actor_error_t actor_message_queue_get(actor_message_queue_t queue,
    actor_message_t* message, actor_time_t timeout);

#endif
