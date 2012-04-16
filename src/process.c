#include <stdio.h>
#include "process.h"

// spawn new process
process_pid process_spawn(process_process_function function) {
    // TODO
    // call process function
    function();

    // return error pid
    return 0;
}
