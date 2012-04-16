#include <stdlib.h>
#include "message.h"

// create new queue
message_queue* message_queue_create(dispatch_queue_t dispatch_queue,
    message_queue_length length) {
    // create new message struct
    message_queue* queue = malloc(sizeof(message_queue));

    // init parameter
    queue->length = length;
    queue->start = 0;
    queue->end = 0;
    queue->dispatch_queue = dispatch_queue;
    queue->messages = malloc(length * sizeof(message_message));

    return queue;
}

// cleanup
void message_queue_cleanup(message_queue* queue) {
    // free message buffer
    free(queue->messages);

    // free queue
    free(queue);
}
