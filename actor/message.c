#include <string.h>
#include "actor.h"

// create new message
actor_error_t actor_message_create(actor_message_t* message,
    actor_message_data_t const data, actor_size_t size) {
    // check valid message pointer
    if (message == NULL) {
        return ACTOR_FAILURE;
    }

    // init message pointer to NULL
    *message = NULL;

    // create message
    actor_message_t newMessage = malloc(sizeof(actor_message_struct));

    // check success
    if (newMessage == NULL) {
        return ACTOR_FAILURE;
    }

    // init struct
    newMessage->next = NULL;
    newMessage->destination = -1;
    newMessage->size = size;
    newMessage->data = NULL;

    // create message data memory
    newMessage->data = malloc(size);

    // check success
    if (newMessage->data == NULL) {
        // release message
        actor_message_release(newMessage);

        return ACTOR_FAILURE;
    }

    // copy message data
    memcpy(newMessage->data, data, size);

    // set message pointer
    *message = newMessage;

    return ACTOR_SUCCESS;
}

actor_error_t actor_message_release(actor_message_t message) {
    // check for valid message
    if (message == NULL) {
        return ACTOR_FAILURE;
    }

    // free message data
    if (message->data != NULL) {
        free(message->data);
    }

    // free memory
    free(message);

    return ACTOR_SUCCESS;
}

// create new queue
actor_error_t actor_message_queue_create(actor_message_queue_t* queue) {
    // check valid queue pointer
    if (queue == NULL) {
        return ACTOR_FAILURE;
    }

    // init queue pointer to NULL
    *queue = NULL;

    // create new message struct
    actor_message_queue_t newQueue = malloc(sizeof(actor_message_queue_struct));

    // check success
    if (newQueue == NULL) {
        return ACTOR_FAILURE;
    }

    // init struct
    newQueue->semaphore_read_write = NULL;
    newQueue->semaphore_messages = NULL;
    newQueue->first = NULL;
    newQueue->last = NULL;

    // create semaphores
    newQueue->semaphore_read_write = dispatch_semaphore_create(1);
    newQueue->semaphore_messages = dispatch_semaphore_create(0);

    // check success
    if ((newQueue->semaphore_read_write == NULL) ||
            (newQueue->semaphore_messages == NULL)) {
        // release message queue
        actor_message_queue_release(newQueue);

        return ACTOR_FAILURE;
    }

    // set queue pointer
    *queue = newQueue;

    return ACTOR_SUCCESS;
}

actor_error_t actor_message_queue_release(actor_message_queue_t queue) {
    // check for valid queue
    if (queue == NULL) {
        return ACTOR_FAILURE;
    }

    // release all messages
    actor_message_t message = NULL;
    do {
        // get message
        actor_message_queue_get(queue, &message, 0.0);

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

    return ACTOR_SUCCESS;
}

// add new message to queue
actor_error_t actor_message_queue_put(actor_message_queue_t queue,
    actor_message_t message) {
    // check for correct input
    if ((queue == NULL) || (message == NULL)) {
        return ACTOR_FAILURE;
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
        message->next = NULL;
    }

    // signal new message
    dispatch_semaphore_signal(queue->semaphore_messages);

    // signal read write access
    dispatch_semaphore_signal(queue->semaphore_read_write);

    return ACTOR_SUCCESS;
}

// get message from queue
actor_error_t actor_message_queue_get(actor_message_queue_t queue,
    actor_message_t* message, actor_time_t timeout) {
    // check for correct input
    if ((queue == NULL) || (timeout < 0.0) || (message == NULL)) {
        return ACTOR_FAILURE;
    }

    // init message pointer to NULL;
    *message = NULL;

    // get message recource
    long err = dispatch_semaphore_wait(queue->semaphore_messages,
        dispatch_time(DISPATCH_TIME_NOW,
            (dispatch_time_t)(timeout * (actor_time_t)NSEC_PER_SEC)));

    // check for timeout
    if (err != 0) {
        return ACTOR_FAILURE;
    }

    // get read acces
    dispatch_semaphore_wait(queue->semaphore_read_write, DISPATCH_TIME_FOREVER);

    // get message
    actor_message_t newMessage = queue->first;

    // set new first message to next
    queue->first = (actor_message_t)newMessage->next;

    // if first is NULL set last to NULL
    if (queue->first == NULL) {
        queue->last = NULL;
    }

    // signal read write access
    dispatch_semaphore_signal(queue->semaphore_read_write);

    // set next element of message to NULL
    newMessage->next = NULL;

    // set message pointer
    *message = newMessage;

    return ACTOR_SUCCESS;
}
