#ifndef MESSAGE_H
#define MESSAGE_H

#include <dispatch/dispatch.h>

// message size
typedef unsigned int message_message_size;

// message struct
typedef struct {
    message_message_size message_size;
    void* message_data;
    struct message_message* next;
} message_message;

// message queue types
typedef unsigned int message_queue_length;
typedef unsigned int message_queue_start;
typedef unsigned int message_queue_end;

// message queue
typedef struct {
    dispatch_queue_t dispatch_queue;
    message_message* first;
    message_message* last;
} message_queue;

// create new message
message_message* message_message_create(void* const data,
    message_message_size size);

// cleanup message
void message_message_cleanup(message_message* message);

// create new queue
message_queue* message_queue_create(dispatch_queue_t dispatch_queue);
message_queue* message_queue_init(message_queue* queue,
    dispatch_queue_t dispatch_queue);

// cleanup queue
void message_queue_cleanup(message_queue* queue);

// add new message to queue
void message_queue_put(message_queue* queue, message_message* message);

// get message from queue
message_message* message_queue_get(message_queue* queue, float timeout);

#endif
