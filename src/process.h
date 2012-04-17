#include <Block.h>
#include "node.h"
#include "message.h"

#ifndef PROCESS_H
#define PROCESS_H

// Process Identifier
typedef unsigned int process_id;

// Process struct
typedef struct {
    process_id pid;
    message_queue* queue;
} process_process;

// Process block signature
typedef void (^process_process_function)(process_process* const self);

// spawn new process
process_process* process_spawn(node_node* node, process_process_function function);

// process cleanup
void process_cleanup(process_process* process);

#endif
