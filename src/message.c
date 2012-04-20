#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "message.h"

// create new message
actor_message_t actor_message_create(void* const data,
    actor_message_size_t size) {
    // create message
    actor_message_t message = malloc(sizeof(actor_message_struct));

    // copy data
    void* dataCopy = malloc(sizeof(char) * size);
    memcpy(dataCopy, data, sizeof(char) * size);

    // save data
    message->message_data = dataCopy;
    message->message_size = size;

    // set next to NULL
    message->next = NULL;

    return message;
}

// cleanup message
void actor_message_cleanup(actor_message_t message) {
    // check for valid message
    if (message == NULL) {
        return;
    }

    free(message->message_data);
}

void actor_message_release(actor_message_t message) {
    // check for valid message
    if (message == NULL) {
        return;
    }

    // cleanup message
    actor_message_cleanup(message);

    // free memory
    free(message);
}

// create new queue
actor_message_queue_t message_queue_create() {
    // create new message struct
    actor_message_queue_t queue = malloc(sizeof(actor_message_queue_struct));

    // init queue
    actor_message_queue_init(queue);

    return queue;
}

// create new queue
actor_message_queue_t actor_message_queue_init(actor_message_queue_t queue) {
    // check for valid queue
    if (queue == NULL) {
        return NULL;
    }

    // init parameter
    queue->first = NULL;
    queue->last = NULL;

    // create semaphores
    queue->semaphore_read_write = dispatch_semaphore_create(1);
    queue->semaphore_messages = dispatch_semaphore_create(0);

    return queue;
}

// cleanup
void actor_message_queue_cleanup(actor_message_queue_t queue) {
    // check for valid queue
    if (queue == NULL) {
        return;
    }

    // get first message
    actor_message_t message = queue->first;

    // pointer to next message
    actor_message_t next = NULL;

    // free all messages
    while (message != NULL) {
        // get next message
        next = (actor_message_t)message->next;

        // release message
        actor_message_release(message);

        // continue to next message
        message = next;
    }

    // release semaphores
    dispatch_release(queue->semaphore_read_write);
    dispatch_release(queue->semaphore_messages);
}

void actor_message_queue_release(actor_message_queue_t queue) {
    // check for valid queue
    if (queue == NULL) {
        return;
    }

    // cleanup queue
    actor_message_queue_cleanup(queue);

    // free queue
    free(queue);
}

// add new message to queue
void actor_message_queue_put(actor_message_queue_t queue, actor_message_t message) {
    // check for correct input
    if ((queue == NULL) || (message == NULL)) {
        return;
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
