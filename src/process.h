#ifndef PROCESS_H
#define PROCESS_H

#include "node.h"
#include "message.h"

// Process Identifier
typedef unsigned int process_id;

// Process struct
typedef struct {
    process_id pid;
    message_queue* queue;
    node_node* process_node;
} process_process;

// Process block signature
typedef void (^process_process_function)(process_process* const self);

// spawn new process
process_process* process_spawn(node_node* node, process_process_function function);

// message sending
message_message* process_message_send(process_process* process, process_id dest_id,
    message_message* message);

// message receive
message_message* process_message_receive(process_process* process, float timeout);

// process cleanup
void process_cleanup(process_process* process);

#endif
