#include <Block.h>

// Process Identifier
typedef unsigned int process_id;

// Process struct
typedef struct {
    process_id pid;
} process_process;

// Process block signature
typedef void (^process_process_function)();

// spawn new process
process_id process_spawn(process_process_function function);
