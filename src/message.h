#include <dispatch/dispatch.h>

#ifndef MESSAGE_H
#define MESSAGE_H

// message id
typedef unsigned int message_id;

// message length
typedef unsigned int message_message_length;

// message struct
typedef struct {
    message_id mid;
    message_message_length message_length;
    char* message_message;
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

// create new queue
message_queue* message_queue_create(dispatch_queue_t dispatch_queue);

// cleanup
void message_queue_cleanup(message_queue* queue);

// add new message to queue
void message_queue_put(message_queue* queue, message_message* message);

// get message from queue
message_message* message_queue_get(message_queue* queue);

#endif
