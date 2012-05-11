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

#include "actor.h"

// create node
actor_error_t actor_node_create(actor_node_t* nodePointer, actor_node_id_t id,
    actor_size_t size) {
    // check valid node pointer
    if (nodePointer == NULL) {
        return ACTOR_ERROR_INVALUE;
    }

    // init node pointer to NULL
    *nodePointer = NULL;

    // create node
    actor_node_t node = malloc(sizeof(actor_node_s));

    // check success
    if (node == NULL) {
        return ACTOR_ERROR_MEMORY;
    }

    // init struct
    node->id = id;
    node->message_queues = NULL;
    node->remote_nodes = NULL;
    node->message_queue_count = size;
    node->message_queue_pos = 0;
    node->process_semaphore = NULL;
    node->message_queue_create_semaphore = NULL;
    node->process_count = 0;

    // create message queues
    node->message_queues = malloc(sizeof(actor_message_queue_t) * size);

    // check success
    if (node->message_queues == NULL) {
        // release node
        actor_node_release(&node);

        return ACTOR_ERROR_MEMORY;
    }

    // create remote node array
    node->remote_nodes = malloc(sizeof(int) * ACTOR_NODE_MAX_REMOTE_NODES);

    // check success
    if (node->remote_nodes == NULL) {
        // release node
        actor_node_release(&node);

        return ACTOR_ERROR_MEMORY;
    }

    // init array
    for (actor_size_t i = 0; i < ACTOR_NODE_MAX_REMOTE_NODES; i++) {
        node->remote_nodes[i] = ACTOR_INVALID_ID;
    }

    // create process semaphore
    node->process_semaphore = dispatch_semaphore_create(0);

    // check success
    if (node->process_semaphore == NULL) {
        // release node
        actor_node_release(&node);

        return ACTOR_ERROR_DISPATCH;
    }

    // create message queue create semaphore
    node->message_queue_create_semaphore = dispatch_semaphore_create(1);

    // check success
    if (node->message_queue_create_semaphore == NULL) {
        // release node
        actor_node_release(&node);

        return ACTOR_ERROR_DISPATCH;
    }

    // set node pointer
    *nodePointer = node;

    return ACTOR_SUCCESS;
}

actor_error_t actor_node_release(actor_node_t* nodePointer) {
    // check for valid node
    if ((nodePointer == NULL) || (*nodePointer == NULL)) {
        return ACTOR_ERROR_INVALUE;
    }

    // get node
    actor_node_t node = *nodePointer;

    // wait for all processes to complete
    dispatch_semaphore_wait(node->process_semaphore, DISPATCH_TIME_FOREVER);

    // release message queues
    if (node->message_queues != NULL) {
        for (actor_size_t i = 0; i < node->message_queue_count; i++) {
            actor_message_queue_release(&node->message_queues[i]);
        }

        free(node->message_queues);
    }

    // release remote node array
    if (node->remote_nodes != NULL) {
        free(node->remote_nodes);
    }

    // release process semaphore
    if (node->process_semaphore != NULL) {
        dispatch_release(node->process_semaphore);
    }

    // release message queue create semaphore
    if (node->message_queue_create_semaphore != NULL) {
        dispatch_release(node->message_queue_create_semaphore);
    }

    // free memory
    free(node);

    // set node pointer to NULL
    *nodePointer = NULL;

    return ACTOR_SUCCESS;
}

// spawn new process
actor_error_t actor_node_spawn_process(actor_node_t node, actor_process_id_t* pid,
    actor_process_function_t function) {
    // check for valid node
    if (node == NULL) {
        return ACTOR_ERROR_INVALUE;
    }

    // error
    actor_error_t error = ACTOR_SUCCESS;

    // init pid to invalid
    if (pid != NULL) {
        *pid = ACTOR_INVALID_ID;
    }

    // create process
    __block actor_process_t process = NULL;
    error = actor_process_create(node, &process);

    // check success
    if (error != ACTOR_SUCCESS) {
        return error;
    }

    // get dispatch queue
    dispatch_queue_t dispatch_queue = dispatch_get_global_queue(
        DISPATCH_QUEUE_PRIORITY_HIGH, 0);

    // check for success
    if (dispatch_queue == NULL) {
        // cleanup
        actor_process_release(&process);

        return ACTOR_ERROR_DISPATCH;
    }

    // invoke new procces
    dispatch_async(dispatch_queue, ^ {
        // call process kernel
        actor_error_t result = function(process);

        // create error message
        actor_process_error_message_s error_message;
        error_message.nid = process->nid;
        error_message.pid = process->pid;
        error_message.error = result;

        // send message
        actor_send(process, process->supervisor_nid,
            process->supervisor_pid, ACTOR_TYPE_ERROR_MESSAGE, &error_message,
            sizeof(actor_process_error_message_s));

        // cleanup process
        actor_process_release(&process);
    });

    // set pid
    if (pid != NULL) {
        *pid = process->pid;
    }

    return ACTOR_SUCCESS;
}

// message sending
actor_error_t actor_node_send_message(actor_node_t node,
    actor_node_id_t destination_nid, actor_process_id_t destination_pid,
    actor_data_type_t type, actor_message_data_t const data, actor_size_t size) {
    // check input
    if ((node == NULL) || (data == NULL) || (type < 0)) {
        return ACTOR_ERROR_INVALUE;
    }

    // error
    actor_error_t error = ACTOR_SUCCESS;

    // check destination nid
    if ((destination_nid < 0) || (destination_nid >= ACTOR_NODE_MAX_REMOTE_NODES)) {
        return ACTOR_ERROR_INVALUE;
    }

    // check destination pid
    if (destination_pid < 0) {
        return ACTOR_ERROR_INVALUE;
    }

    // create message
    actor_message_t message = NULL;
    if (actor_message_create(&message, type, data, size) != ACTOR_SUCCESS) {
        return ACTOR_ERROR_MEMORY;
    }

    // set message destination
    message->destination_nid = destination_nid;
    message->destination_pid = destination_pid;

    // destination message queue
    actor_message_queue_t* queue = NULL;

    // check node id
    if (destination_nid == node->id) {
        // get message queue
        error = actor_node_get_message_queue(node, &queue, destination_pid);

        // check success
        if (error != ACTOR_SUCCESS) {
            // release message
            actor_message_release(&message);

            return error;
        }
    }
    else {
        // get remote node message queue
        error = actor_node_get_message_queue(node, &queue,
            node->remote_nodes[destination_nid]);

        // check success
        if (error != ACTOR_SUCCESS) {
            // release message
            actor_message_release(&message);

            return error;
        }
    }

    // enqueue message
    return actor_message_queue_put(*queue, message);
}

// get free message queue
actor_error_t actor_node_get_free_message_queue(actor_node_t node,
    actor_message_queue_t* queue, actor_process_id_t* pid) {
    // check for correct input
    if ((node == NULL) || (pid == NULL) || (queue == NULL)) {
        return ACTOR_ERROR_INVALUE;
    }

    // error
    actor_error_t error = ACTOR_SUCCESS;

    // init queue and pid pointer
    *queue = NULL;
    *pid = ACTOR_INVALID_ID;

    // get message queue create access
    dispatch_semaphore_wait(node->message_queue_create_semaphore,
        DISPATCH_TIME_FOREVER);

    // get possible id
    actor_process_id_t id = node->message_queue_pos;

    // check for correct id
    if ((id >= node->message_queue_count) ||
            (node->message_queues[id] != NULL)) {
        // look for first free queue
        for (actor_size_t i = 0; i < node->message_queue_count; i++) {
            // check for used queue
            if (node->message_queues[i] == NULL) {
                // set new id
                id = i;

                break;
            }
        }

        // check new id
        if (id == node->message_queue_pos) {
            // release message queue create access
            dispatch_semaphore_signal(node->message_queue_create_semaphore);

            return ACTOR_ERROR_TOO_MANY_PROCESSES;
        }

        // set new id
        *pid = id;
        node->message_queue_pos = id + 1;
    }
    else {
        // set id
        *pid = node->message_queue_pos;

        // increment count
        node->message_queue_pos++;
    }

    // create new message queue
    actor_message_queue_t newQueue = NULL;
    error = actor_message_queue_create(&newQueue);

    // check success
    if (error != ACTOR_SUCCESS) {
        // release message queue create access
        dispatch_semaphore_signal(node->message_queue_create_semaphore);

        return error;
    }

    // register queue
    node->message_queues[*pid] = newQueue;

    // increment process counter
    node->process_count++;
    dispatch_semaphore_wait(node->process_semaphore, DISPATCH_TIME_NOW);

    // release message queue create access
    dispatch_semaphore_signal(node->message_queue_create_semaphore);

    // set queue pointer
    *queue = newQueue;

    return ACTOR_SUCCESS;
}

// get message queue for id
actor_error_t actor_node_get_message_queue(actor_node_t node,
    actor_message_queue_t** queue, actor_process_id_t pid) {
    // check for valid input
    if ((node == NULL) || (queue == NULL)) {
        return ACTOR_ERROR_INVALUE;
    }

    // check for correct pid
    if ((pid >= node->message_queue_count) || (pid < 0)) {
        return ACTOR_ERROR_INVALUE;
    }

    // set queue pointer
    *queue = &node->message_queues[pid];

    return ACTOR_SUCCESS;
}

// release message queue
actor_error_t actor_node_message_queue_release(actor_node_t node,
    actor_process_id_t pid) {
    // check for correct pid
    if ((pid >= node->message_queue_count) || (pid < 0)) {
        return ACTOR_ERROR_INVALUE;
    }

    // release message queue
    actor_message_queue_release(&node->message_queues[pid]);

    // set queue pointer to NULL
    node->message_queues[pid] = NULL;

    // decrement process counter
    node->process_count--;

    // send signal if no process left
    if (node->process_count == 0) {
        dispatch_semaphore_signal(node->process_semaphore);
    }

    return ACTOR_SUCCESS;
}

// connect to remote node
actor_error_t actor_node_connect(actor_node_t node, actor_node_id_t* nid,
    const char* host_name, unsigned int host_port, const char* key) {
    // check for valid node
    if (node == NULL) {
        return ACTOR_ERROR_INVALUE;
    }

    // connect to remote
    return actor_distributer_connect_to_node(node, nid, host_name, host_port, key);
}

// listen for incomming connection
actor_error_t actor_node_listen(actor_node_t node, actor_node_id_t* nid,
    unsigned int port, const char* key) {
    // check for valid node
    if (node == NULL) {
        return ACTOR_ERROR_INVALUE;
    }

    // start listening
    return actor_distributer_listen(node, nid, port, key);
}

// close connection
actor_error_t actor_node_disconnect(actor_node_t node, actor_node_id_t nid) {
    // check for valid node
    if (node == NULL) {
        return ACTOR_ERROR_INVALUE;
    }

    // disconnect
    return actor_distributer_disconnect_from_node(node, nid);
}
