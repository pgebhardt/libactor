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

#include "../include/actor.h"

// create process
actor_error_t actor_process_create(actor_node_t node, actor_process_t* processPointer) {
    // check valid input
    if ((node == NULL) || (processPointer == NULL)) {
        return ACTOR_ERROR_INVALUE;
    }

    // error
    actor_error_t error = ACTOR_SUCCESS;

    // init process pointer to NULL
    *processPointer = NULL;

    // create process struct
    actor_process_t process = malloc(sizeof(actor_process_s));

    // check success
    if (process == NULL) {
        return ACTOR_ERROR_MEMORY;
    }

    // init struct
    process->pid = ACTOR_INVALID_ID;
    process->nid = node->id;
    process->node = node;
    process->supervisor_nid = ACTOR_INVALID_ID;
    process->supervisor_pid = ACTOR_INVALID_ID;
    process->message_queue = NULL;
    process->sleep_semaphore = NULL;

    // create sleep semaphore
    process->sleep_semaphore = dispatch_semaphore_create(0);

    // check for success
    if (process->sleep_semaphore == NULL) {
        // release process
        actor_process_release(&process);

        return ACTOR_ERROR_DISPATCH;
    }

    // get free message queue
    error = actor_node_get_free_message_queue(node,
        &process->message_queue, &process->pid);

    // check success
    if (error != ACTOR_SUCCESS) {
        // release process
        actor_process_release(&process);

        return error;
    }

    // set process pointer
    *processPointer = process;

    return ACTOR_SUCCESS;
}

// release process
actor_error_t actor_process_release(actor_process_t* processPointer) {
    // check for valid process
    if ((processPointer == NULL) || (*processPointer == NULL)) {
        return ACTOR_ERROR_INVALUE;
    }

    // get process
    actor_process_t process = *processPointer;

    // release sleep semaphore
    if (process->sleep_semaphore != NULL) {
        dispatch_release(process->sleep_semaphore);
    }

    // release message queue
    if (process->message_queue != NULL) {
        actor_node_message_queue_release(process->node, process->pid);
    }

    // free process memory
    free(process);

    // set process pointer to NULL
    *processPointer = NULL;

    return ACTOR_SUCCESS;
}

// message receive
actor_error_t actor_process_receive_message(actor_process_t process, actor_message_t* message,
    actor_time_t timeout) {
    // check for correct input
    if ((process == NULL) || (timeout < 0.0) || (message == NULL)) {
        return ACTOR_ERROR_INVALUE;
    }

    // get message
    return actor_message_queue_get(process->message_queue, message, timeout);
}

// sleep
actor_error_t actor_process_sleep(actor_process_t process, actor_time_t time) {
    // check for correct input
    if ((process == NULL) || (time < 0.0)) {
        return ACTOR_ERROR_INVALUE;
    }

    // wait for timeout
    dispatch_semaphore_wait(process->sleep_semaphore,
        dispatch_time(DISPATCH_TIME_NOW,
            (dispatch_time_t)(time * (double)NSEC_PER_SEC)));

    return ACTOR_SUCCESS;
}

// link
actor_error_t actor_process_link(actor_process_t process,
    actor_node_id_t supervisor_nid, actor_process_id_t supervisor_pid) {
    // check valid process
    if (process == NULL) {
        return ACTOR_ERROR_INVALUE;
    }

    // check supervisor id
    if ((supervisor_pid < 0) || (supervisor_nid < 0) ||
        ((supervisor_nid == process->nid) && (supervisor_pid == process->pid))) {
        return ACTOR_ERROR_INVALUE;
    }

    // set supervisor
    process->supervisor_nid = supervisor_nid;
    process->supervisor_pid = supervisor_pid;

    return ACTOR_SUCCESS;
}

actor_error_t actor_process_unlink(actor_process_t process) {
    // check valid process
    if (process == NULL) {
        return ACTOR_ERROR_INVALUE;
    }

    // unlink
    process->supervisor_nid = ACTOR_INVALID_ID;
    process->supervisor_pid = ACTOR_INVALID_ID;

    return ACTOR_SUCCESS;
}
