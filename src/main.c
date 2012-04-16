#include <stdio.h>
#include <time.h>
#include "process.h"

int main(int argc, char* argv[]) {
    // spawn dummy process
    process_process* process = process_spawn(^{
            // print some stuff
            printf("Hallo du da, was geht?\n");
        });

    // output pid
    printf("Pid: %d\n", process->pid);

    // sleep a bit
    sleep(1);

    // cleanup
    process_cleanup(process);

    return 0;
}
