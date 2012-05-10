#include <string.h>
#include "actor.h"

// create new message
actor_error_t actor_message_create(actor_message_t* messagePointer,
    actor_data_type_t type, actor_message_data_t const data, actor_size_t size) {
    // check input
    if ((messagePointer == NULL) || (type < 0) || (data == NULL)) {
        return ACTOR_ERROR_INVALUE;
    }

    // init message pointer to NULL
    *messagePointer = NULL;

    // create message
    actor_message_t message = malloc(sizeof(actor_message_s));

    // check success
    if (message == NULL) {
        return ACTOR_ERROR_MEMORY;
    }

    // init struct
    message->next = NULL;
    message->destination_nid = ACTOR_INVALID_ID;
    message->destination_pid = ACTOR_INVALID_ID;
    message->size = size;
    message->data = NULL;
    message->type = type;

    // create message data memory
    message->data = malloc(size);

    // check success
    if (message->data == NULL) {
        // release message
        actor_message_release(&message);

        return ACTOR_ERROR_MEMORY;
    }

    // copy message data
    memcpy(message->data, data, size);

    // set message pointer
    *messagePointer = message;

    return ACTOR_SUCCESS;
}

actor_error_t actor_message_release(actor_message_t* messagePointer) {
    // check for valid message
    if ((messagePointer == NULL) || (*messagePointer == NULL)) {
        return ACTOR_ERROR_INVALUE;
    }

    // get message
    actor_message_t message = *messagePointer;

    // free message data
    if (message->data != NULL) {
        free(message->data);
    }

    // free memory
    free(message);

    // set message pointer to NULL
    *messagePointer = NULL;

    return ACTOR_SUCCESS;
}

// create new queue
actor_error_t actor_message_queue_create(actor_message_queue_t* queuePointer) {
    // check valid queue pointer
    if (queuePointer == NULL) {
        return ACTOR_ERROR_INVALUE;
    }

    // init queue pointer to NULL
    *queuePointer = NULL;

    // create new message struct
    actor_message_queue_t queue = malloc(sizeof(actor_message_queue_s));

    // check success
    if (queue == NULL) {
        return ACTOR_ERROR_MEMORY;
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
        actor_message_queue_release(&queue);

        return ACTOR_ERROR_DISPATCH;
    }

    // set queue pointer
    *queuePointer = queue;

    return ACTOR_SUCCESS;
}

actor_error_t actor_message_queue_release(actor_message_queue_t* queuePointer) {
    // check for valid queue
    if ((queuePointer == NULL) || (*queuePointer == NULL)) {
        return ACTOR_ERROR_INVALUE;
    }

    // get queue
    actor_message_queue_t queue = *queuePointer;

    // release all messages
    actor_message_t message = NULL;
    do {
        // get message
        actor_message_queue_get(queue, &message, 0.0);

        // release message
        actor_message_release(&message);
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

    // set queue pointer to NULL;
    *queuePointer = NULL;

    return ACTOR_SUCCESS;
}

// add new message to queue
actor_error_t actor_message_queue_put(actor_message_queue_t queue,
    actor_message_t message) {
    // check for correct input
    if ((queue == NULL) || (message == NULL)) {
        return ACTOR_ERROR_INVALUE;
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
        queue->last->next = (struct actor_message_s*)message;
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
        return ACTOR_ERROR_INVALUE;
    }

    // init message pointer to NULL;
    *message = NULL;

    // get message recource
    long err = dispatch_semaphore_wait(queue->semaphore_messages,
        dispatch_time(DISPATCH_TIME_NOW,
            (dispatch_time_t)(timeout * (actor_time_t)NSEC_PER_SEC)));

    // check for timeout
    if (err != 0) {
        return ACTOR_ERROR_TIMEOUT;
    }

    // get read acces
    dispatch_semaphore_wait(queue->semaphore_read_write, DISPATCH_TIME_FOREVER);

    // get message
    actor_message_t newMessage = queue->first;

    // check message
    if (newMessage == NULL) {
        // signal read write access
        dispatch_semaphore_signal(queue->semaphore_read_write);

        return ACTOR_ERROR_MESSAGE_PASSING;
    }

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
