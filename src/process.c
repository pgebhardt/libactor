#include <stdlib.h>
#include "node.h"
#include "process.h"

// create process
actor_process_t actor_process_create(actor_process_id_t pid, actor_node_t node,
    actor_message_queue_t message_queue) {
    // check parameter
    if ((pid < 0) || (node == NULL) || (message_queue == NULL)) {
        return NULL;
    }

    // create sleep semaphore
    dispatch_semaphore_t sleep_semaphore = dispatch_semaphore_create(0);

    // check for success
    if (sleep_semaphore == NULL) {
        return NULL;
    }

    // create process struct
    actor_process_t process = malloc(sizeof(actor_process_struct));

    // check for success
    if (process == NULL) {
        return NULL;
    }

    // save attributes
    process->pid = pid;
    process->message_queue = message_queue;
    process->node = node;
    process->sleep_semaphore = sleep_semaphore;

    return process;
}

// message sendig
actor_message_t actor_message_send(actor_process_t process,
    actor_process_id_t dest_id, actor_message_t message) {
    // check for valid process
    if (process == NULL) {
        return NULL;
    }

    // check for correct dest_id
    if (dest_id >= process->node->process_size) {
        return NULL;
    }

    // get destination message queue
    actor_message_queue_t dest_queue = actor_node_message_queue_get(process->node,
        dest_id);

    // check for succes
    if (dest_queue == NULL) {
        return NULL;
    }

    // enqueue message
    actor_message_queue_put(dest_queue, message);

    return message;
}

// message receive
actor_message_t actor_message_receive(actor_process_t process,
    double timeout) {
    // check for correct input
    if ((process == NULL) || (timeout < 0.0)) {
        return NULL;
    }

    // get message
    return actor_message_queue_get(process->message_queue, timeout);
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

    // signal semaphore
    // dispatch_semaphore_signal(process->sleep_semaphore);
}

// process cleanup
void actor_process_cleanup(actor_process_t process) {
    // check for valid process
    if (process == NULL) {
        return;
    }

    // release queue
    actor_node_message_queue_release(process->node, process->pid);

    // release sleep semaphore
    dispatch_release(process->sleep_semaphore);
}

void actor_process_release(actor_process_t process) {
    // check for valid process
    if (process == NULL) {
        return;
    }

    // cleanup process
    actor_process_cleanup(process);

    // free process memory
    free(process);
}
