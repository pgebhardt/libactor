#include <Block.h>

// Process Identifier
typedef unsigned int process_pid;

// Process struct
typedef struct {
    process_pid pid;
} process_process;

// Process block signature
typedef void (^process_process_function)();

// spawn new process
process_pid process_spawn(process_process_function function);
