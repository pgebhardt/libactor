#include <string.h>
#include "actor.h"

// create new message
actor_message_t actor_message_create(actor_message_data_t const data,
    actor_size_t size) {
    // create message
    actor_message_t message = malloc(sizeof(actor_message_struct));

    // check success
    if (message == NULL) {
        return NULL;
    }

    // init struct
    message->next = NULL;
    message->size = size;
    message->data = NULL;

    // create message data memory
    message->data = malloc(size);

    // check success
    if (message->data == NULL) {
        // release message
        actor_message_release(message);

        return NULL;
    }

    // copy message data
    memcpy(message->data, data, size);

    return message;
}

void actor_message_release(actor_message_t message) {
    // check for valid message
    if (message == NULL) {
        return;
    }

    // free message data
    if (message->data != NULL) {
        free(message->data);
    }

    // free memory
    free(message);
}

// create new queue
actor_message_queue_t actor_message_queue_create() {
    // create new message struct
    actor_message_queue_t queue = malloc(sizeof(actor_message_queue_struct));

    // check success
    if (queue == NULL) {
        return NULL;
    }

    // init struct
    queue->semaphore_read_write = NULL;
    queue->semaphore_messages = NULL;
    queue->first = NULL;
    queue->last = NULL;

    // create semaphores
    queue->semaphore_read_write = dispatch_semaphore_create(1);
    queue->semaphore_messages = dispatch_semaphore_create(0);

    // check success
    if ((queue->semaphore_read_write == NULL) ||
            (queue->semaphore_messages == NULL)) {
        // release message queue
        actor_message_queue_release(queue);

        return NULL;
    }

    return queue;
}

void actor_message_queue_release(actor_message_queue_t queue) {
    // check for valid queue
    if (queue == NULL) {
        return;
    }

    // release all messages
    actor_message_t message = NULL;
    do {
        // get message
        message = actor_message_queue_get(queue, 0.0);

        // release message
        actor_message_release(message);
    } while (message != NULL);

    // release semaphores
    if (queue->semaphore_read_write != NULL) {
        dispatch_release(queue->semaphore_read_write);
    }

    if (queue->semaphore_messages != NULL) {
        dispatch_release(queue->semaphore_messages);
    }

    // free queue
    free(queue);
}

// add new message to queue
actor_message_t actor_message_queue_put(actor_message_queue_t queue,
    actor_message_t message) {
    // check for correct input
    if ((queue == NULL) || (message == NULL)) {
        return NULL;
    }

    // get write access
    dispatch_semaphore_wait(queue->semaphore_read_write, DISPATCH_TIME_FOREVER);

    // check if first message is NULL
    if (queue->first == NULL) {
        // set new message as first and last
        queue->first = message;
        queue->last = message;
        message->next = NULL;
    }
    else {
        // set new message as last
        queue->last->next = (struct actor_message_struct*)message;
        queue->last = message;
    }

    // signal new message
    dispatch_semaphore_signal(queue->semaphore_messages);

    // signal read write access
    dispatch_semaphore_signal(queue->semaphore_read_write);

    // return message on success
    return message;
}

// get message from queue
actor_message_t actor_message_queue_get(actor_message_queue_t queue,
    double timeout) {
    // check for correct input
    if ((queue == NULL) || (timeout < 0.0)) {
        return NULL;
    }

    // get message recource
    long err = dispatch_semaphore_wait(queue->semaphore_messages,
        dispatch_time(DISPATCH_TIME_NOW,
            (dispatch_time_t)(timeout * (double)NSEC_PER_SEC)));

    // check for timeout
    if (err != 0) {
        return NULL;
    }

    // get read acces
    dispatch_semaphore_wait(queue->semaphore_read_write, DISPATCH_TIME_FOREVER);

    // get message
    actor_message_t message = queue->first;

    // set new first message to next
    queue->first = (actor_message_t)message->next;

    // if first is NULL set last to NULL
    if (queue->first == NULL) {
        queue->last = NULL;
    }

    // signal read write access
    dispatch_semaphore_signal(queue->semaphore_read_write);

    // set next element of message to NULL
    message->next = NULL;

    return message;
}
