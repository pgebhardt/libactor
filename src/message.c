#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "message.h"

// create new message
message_message* message_message_create(void* const data,
    message_message_size size) {
    // create message
    message_message* message = malloc(sizeof(message_message));

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
void message_message_cleanup(message_message* message) {
    // check for valid message
    if (message == NULL) {
        return;
    }

    free(message->message_data);
}

void message_message_release(message_message* message) {
    // check for valid message
    if (message == NULL) {
        return;
    }

    // cleanup message
    message_message_cleanup(message);

    // free memory
    free(message);
}

// create new queue
message_queue* message_queue_create() {
    // create new message struct
    message_queue* queue = malloc(sizeof(message_queue));

    // init queue
    message_queue_init(queue);

    return queue;
}

// create new queue
message_queue* message_queue_init(message_queue* queue) {
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
void message_queue_cleanup(message_queue* queue) {
    // check for valid queue
    if (queue == NULL) {
        return;
    }

    // get first message
    message_message* message = queue->first;

    // pointer to next message
    message_message* next = NULL;

    // free all messages
    while (message != NULL) {
        // get next message
        next = (message_message*)message->next;

        // release message
        message_message_release(message);

        // continue to next message
        message = next;
    }

    // release semaphores
    dispatch_release(queue->semaphore_read_write);
    dispatch_release(queue->semaphore_messages);
}

void message_queue_release(message_queue* queue) {
    // check for valid queue
    if (queue == NULL) {
        return;
    }

    // cleanup queue
    message_queue_cleanup(queue);

    // free queue
    free(queue);
}

// add new message to queue
void message_queue_put(message_queue* queue, message_message* message) {
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
        queue->last->next = (struct message_message*)message;
        queue->last = message;
    }

    // signal new message
    dispatch_semaphore_signal(queue->semaphore_messages);

    // signal read write access
    dispatch_semaphore_signal(queue->semaphore_read_write);
}

// get message from queue
message_message* message_queue_get(message_queue* queue,
    double timeout) {
    // check for correct input
    if ((queue == NULL) || (timeout < 0.0)) {
        return NULL;
    }

    // TODO: correct timeout
    // get message recource
    dispatch_semaphore_wait(queue->semaphore_messages, DISPATCH_TIME_FOREVER);

    // get read acces
    dispatch_semaphore_wait(queue->semaphore_read_write, DISPATCH_TIME_FOREVER);

    // get message
    message_message* message = queue->first;

    // set new first message to next
    queue->first = (message_message*)message->next;

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
