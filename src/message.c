#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
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
    // check for valid queue
    if ((queue == NULL) || (dispatch_queue == NULL)) {
        return NULL;
    }

    // init parameter
    queue->first = NULL;
    queue->last = NULL;
    queue->dispatch_queue = dispatch_queue;

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

    // release dispatch queue
    dispatch_release(queue->dispatch_queue);
    queue->dispatch_queue = NULL;
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

    // dispatch write
    dispatch_async(queue->dispatch_queue, ^ {
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
message_message* message_queue_get(message_queue* queue,
    double timeout) {
    // check for correct input
    if ((queue == NULL) || (timeout < 0.0)) {
        return NULL;
    }

    // convert timeout
    message_queue_timeout t = (message_queue_timeout)(timeout * 1e6 / MESSAGE_MICROSECONDS_WAIT);

    // check for message
    while (queue->first == NULL) {
        // check current timeout
        if (t == 0) {
            return NULL;
        }

        // sleep a bit
        usleep(MESSAGE_MICROSECONDS_WAIT);

        // decement timeout
        t -= 1;
    }

    // get message
    message_message* message = queue->first;

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
