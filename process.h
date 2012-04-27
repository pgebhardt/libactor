#ifndef PROCESS_H
#define PROCESS_H

// Process struct
typedef struct {
    actor_process_id_t nid;
    actor_node_id_t pid;
    actor_node_t node;
    actor_node_id_t supervisor_nid;
    actor_process_id_t supervisor_pid;
    actor_message_queue_t message_queue;
    dispatch_semaphore_t sleep_semaphore;
} actor_process_s;
typedef actor_process_s* actor_process_t;

// process error message
typedef struct {
    actor_node_id_t nid;
    actor_process_id_t pid;
    actor_error_t error;
} actor_process_error_message_s;
typedef actor_process_error_message_s* actor_process_error_message_t;

// Process block signature
typedef actor_error_t (^actor_process_function_t)(actor_process_t const self);

// create process
actor_error_t actor_process_create(actor_node_t node, actor_process_t* process);

// release process
actor_error_t actor_process_release(actor_process_t process);

// sleep
actor_error_t actor_process_sleep(actor_process_t process, actor_time_t time);

// link
actor_error_t actor_process_link(actor_process_t process,
    actor_node_id_t supervisor_nid, actor_process_id_t supervisor_pid);

// unlink
actor_error_t actor_process_unlink(actor_process_t process);

#endif
