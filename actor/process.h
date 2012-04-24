#ifndef PROCESS_H
#define PROCESS_H

// Process struct
typedef struct {
    actor_process_id_t pid;
    actor_message_queue_t message_queue;
    dispatch_semaphore_t sleep_semaphore;
    actor_node_t node;
} actor_process_struct;
typedef actor_process_struct* actor_process_t;

// Process block signature
typedef void (^actor_process_function_t)(actor_process_t const self);

// create process
actor_process_t actor_process_create(actor_node_t node);

// release process
void actor_process_release(actor_process_t process);

// sleep
void actor_process_sleep(actor_process_t process, double time);

#endif
