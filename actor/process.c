#include "actor.h"

// create process
actor_process_t actor_process_create(actor_node_t node) {
    // check valid node
    if (node == NULL) {
        return NULL;
    }

    // create process struct
    actor_process_t process = malloc(sizeof(actor_process_struct));

    // check success
    if (process == NULL) {
        return NULL;
    }

    // init struct
    process->pid = -1;
    process->message_queue = NULL;
    process->sleep_semaphore = NULL;
    process->node = node;
    process->supervisor_nid = -1;
    process->supervisor_pid = -1;

    // create sleep semaphore
    process->sleep_semaphore = dispatch_semaphore_create(0);

    // check for success
    if (process->sleep_semaphore == NULL) {
        // release process
        actor_process_release(process);

        return NULL;
    }

    // get free message queue
    process->message_queue = actor_node_message_queue_get_free(node, &process->pid);

    // check success
    if (process->message_queue == NULL) {
        // release process
        actor_process_release(process);

        return NULL;
    }

    return process;
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
