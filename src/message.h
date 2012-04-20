#ifndef MESSAGE_H
#define MESSAGE_H

#include <dispatch/dispatch.h>

// message size
typedef unsigned int actor_message_size_t;

// message struct
typedef struct {
    struct actor_message_struct* next;
    actor_message_size_t message_size;
    void* message_data;
} actor_message_struct;
typedef actor_message_struct* actor_message_t;

// message queue
typedef struct {
    dispatch_semaphore_t semaphore_read_write;
    dispatch_semaphore_t semaphore_messages;
    actor_message_t first;
    actor_message_t last;
} actor_message_queue_struct;
typedef actor_message_queue_struct* actor_message_queue_t;

// create new message
actor_message_t actor_message_create(void* const data,
    actor_message_size_t size);

// cleanup message
void actor_message_cleanup(actor_message_t message);
void actor_message_release(actor_message_t message);

// create new queue
actor_message_queue_t actor_message_queue_create();
actor_message_queue_t actor_message_queue_init(actor_message_queue_t queue);

// cleanup queue
void actor_message_queue_cleanup(actor_message_queue_t queue);
void actor_message_queue_release(actor_message_queue_t queue);

// add new message to queue
void actor_message_queue_put(actor_message_queue_t queue, actor_message_t message);

// get message from queue
// TODO: timeout type
actor_message_t actor_message_queue_get(actor_message_queue_t queue, double timeout);

#endif
