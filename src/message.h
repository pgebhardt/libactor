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
} message_message;

// message queue types
typedef unsigned int message_queue_length;
typedef unsigned int message_queue_start;
typedef unsigned int message_queue_end;

// message queue
typedef struct {
    message_queue_length length;
    message_queue_start start;
    message_queue_end end;
    dispatch_queue_t dispatch_queue;
    message_message* messages;
} message_queue;

// create new queue
message_queue* message_queue_create(dispatch_queue_t dispatch_queue,
    message_queue_length length);

// cleanup
void message_queue_cleanup(message_queue* queue);

#endif
