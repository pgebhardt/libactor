#include "actor.h"

// create process
actor_error_t actor_process_create(actor_node_t node, actor_process_t* process) {
    // check valid input
    if ((node == NULL) || (process == NULL)) {
        return ACTOR_FAILURE;
    }

    // init process pointer to NULL
    *process = NULL;

    // create process struct
    actor_process_t newProcess = malloc(sizeof(actor_process_struct));

    // check success
    if (newProcess == NULL) {
        return ACTOR_FAILURE;
    }

    // init struct
    newProcess->pid = ACTOR_INVALID_ID;
    newProcess->message_queue = NULL;
    newProcess->sleep_semaphore = NULL;
    newProcess->node = node;
    newProcess->supervisor_nid = ACTOR_INVALID_ID;
    newProcess->supervisor_pid = ACTOR_INVALID_ID;

    // create sleep semaphore
    newProcess->sleep_semaphore = dispatch_semaphore_create(0);

    // check for success
    if (newProcess->sleep_semaphore == NULL) {
        // release process
        actor_process_release(newProcess);

        return ACTOR_FAILURE;
    }

    // get free message queue
    if (actor_node_get_free_message_queue(node,
        &newProcess->message_queue, &newProcess->pid) != ACTOR_SUCCESS) {
        // release process
        actor_process_release(newProcess);

        return ACTOR_FAILURE;
    }

    // set process pointer
    *process = newProcess;

    return ACTOR_SUCCESS;
}

actor_error_t actor_process_release(actor_process_t process) {
    // check for valid process
    if (process == NULL) {
        return ACTOR_FAILURE;
    }

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

    return ACTOR_SUCCESS;
}

// sleep
actor_error_t actor_process_sleep(actor_process_t process, actor_time_t time) {
    // check for correct input
    if ((process == NULL) || (time < 0.0)) {
        return ACTOR_FAILURE;
    }

    // wait for timeout
    dispatch_semaphore_wait(process->sleep_semaphore,
        dispatch_time(DISPATCH_TIME_NOW,
            (dispatch_time_t)(time * (double)NSEC_PER_SEC)));

    return ACTOR_SUCCESS;
}

// link
actor_error_t actor_process_link(actor_process_t process, actor_node_id_t supervisor_nid,
    actor_process_id_t supervisor_pid) {
    // check valid process
    if (process == NULL) {
        return ACTOR_FAILURE;
    }

    // check supervisor id
    if ((supervisor_pid < 0) || (supervisor_nid < 0) ||
        ((supervisor_nid == process->node->nid) && (supervisor_pid == process->pid))) {
        return ACTOR_FAILURE;
    }

    // set supervisor
    process->supervisor_nid = supervisor_nid;
    process->supervisor_pid = supervisor_pid;

    return ACTOR_SUCCESS;
}

actor_error_t actor_process_unlink(actor_process_t process) {
    // check valid process
    if (process == NULL) {
        return ACTOR_FAILURE;
    }

    // unlink
    process->supervisor_nid = -1;
    process->supervisor_pid = -1;

    return ACTOR_SUCCESS;
}
