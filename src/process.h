#ifndef PROCESS_H
#define PROCESS_H

#include <dispatch/dispatch.h>
#include "message.h"
#include "node.h"

// Process Identifier
typedef int process_id;

// Process struct
typedef struct {
    process_id pid;
    message_queue* message_queue;
    dispatch_queue_t dispatch_queue;
    node_node* node;
} process_process;

// Process block signature
typedef void (^process_process_function)(process_process* const self);

// message sending
message_message* process_message_send(process_process* process, process_id dest_id,
    message_message* message);

// message receive
message_message* process_message_receive(process_process* process,
    double timeout);

// process cleanup
void process_cleanup(process_process* process);
void process_release(process_process* process);

#endif
