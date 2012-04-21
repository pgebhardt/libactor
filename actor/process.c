#include "actor.h"

// create process
actor_process_t actor_process_create(actor_process_id_t pid, actor_node_t node,
    actor_message_queue_t message_queue) {
    // check parameter
    if ((pid < 0) || (node == NULL) || (message_queue == NULL)) {
        return NULL;
    }

    // create process struct
    actor_process_t process = malloc(sizeof(actor_process_struct));

    // check success
    if (process == NULL) {
        return NULL;
    }

    // init struct
    process->pid = pid;
    process->message_queue = message_queue;
    process->sleep_semaphore = NULL;
    process->node = node;

    // create sleep semaphore
    process->sleep_semaphore = dispatch_semaphore_create(0);

    // check for success
    if (process->sleep_semaphore == NULL) {
        // release process
        actor_process_release(process);

        return NULL;
    }

    return process;
}

void actor_process_release(actor_process_t process) {
    // check for valid process
    if (process == NULL) {
        return;
    }

    // release sleep semaphore
    if (process->sleep_semaphore != NULL) {
        dispatch_release(process->sleep_semaphore);
    }

    // free process memory
    free(process);
}

// sleep
void actor_process_sleep(actor_process_t process, double time) {
    // check for correct input
    if ((process == NULL) || (time < 0.0)) {
        return;
    }

    // wait for timeout
    dispatch_semaphore_wait(process->sleep_semaphore,
        dispatch_time(DISPATCH_TIME_NOW,
            (dispatch_time_t)(time * (double)NSEC_PER_SEC)));
}

