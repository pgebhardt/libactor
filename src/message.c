#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "message.h"

// create new message
message_message* message_message_create(void* const data,
    message_message_size size) {
    // copy data
    void* dataCopy = malloc(sizeof(char) * size);
    memcpy(dataCopy, data, sizeof(char) * size);

    // create message
    message_message* message = malloc(sizeof(message_message));

    // save data
    message->message_data = dataCopy;
    message->message_size = size;

    return message;
}

// cleanup message
void message_message_cleanup(message_message* message) {
    // check for valid message
    if (message != NULL) {
        free(message->message_data);
        free(message);
    }
}

// create new queue
message_queue* message_queue_create(dispatch_queue_t dispatch_queue) {
    // create new message struct
    message_queue* queue = malloc(sizeof(message_queue));

    // init queue
    message_queue_init(queue, dispatch_queue);

    return queue;
}

// create new queue
message_queue* message_queue_init(message_queue* queue,
    dispatch_queue_t dispatch_queue) {
    // init parameter
    queue->dispatch_queue = dispatch_queue;
    queue->first = NULL;
    queue->last = NULL;

    return queue;
}

// cleanup
void message_queue_cleanup(message_queue* queue) {
    // get first message
    message_message* message = queue->first;

    // pointer to next message
    message_message* next = NULL;

    // free all messages
    while (message != NULL) {
        // get next message
        next = (message_message*)message->next;

        // free message
        free(message);

        // continue to next message
        message = next;
    }

    // free queue
    free(queue);
}

// add new message to queue
void message_queue_put(message_queue* queue, message_message* message) {
    // dispatch sync
    dispatch_sync(queue->dispatch_queue, ^{
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
    });
}

// get message from queue
message_message* message_queue_get(message_queue* queue, float timeout) {
    // check for message
    while (queue->first == NULL) {
        // check current timeout
        if (timeout <= 0.0f) {
            return NULL;
        }

        // sleep a bit
        usleep(100);

        // decement timeout
        timeout -= 0.1e-3f;
    }

    // get message
    message_message* message = queue-> first;

    // set new first message to next
    queue->first = (message_message*)message->next;

    // if first is NULL set last to NULL
    if (queue->first == NULL) {
        queue->last = NULL;
    }

    // set next element of message to NULL
    message->next = NULL;

    return message;
}
