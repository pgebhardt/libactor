#include <stdio.h>
#include "process.h"

int main(int argc, char* argv[]) {
    // spawn dummy process
    process_id pid = process_spawn(^{
            // print some stuff
            printf("Hallo du da, was geht?\n");
        });

    // output pid
    printf("Pid: %d\n", pid);

    return 0;
}
