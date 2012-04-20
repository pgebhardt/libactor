#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "node.h"
#include "message.h"
#include "process.h"

void main_process(process_process* self) {
    // process id memory
    int processes = 100;
    process_id* pids = malloc(processes * sizeof(process_id));

    // process function
    process_process_function function = ^(process_process* s) {
        printf("I'm procces %d!\n", s->pid);
    };

    // create processes
    for (int i = 0; i < processes; i++) {
        node_process_spawn(self->node, function);
    }

    // wait
    sleep(2);
}

int main(int argc, char* argv[]) {
    // create node
    node_node* node = node_create(0, 10000);

    // time variables
    clock_t start, end;

    // get start time
    start = clock();

    // start main process
    node_main_process(node, ^(process_process* self) {
            main_process(self);
        });

    // get end time
    end = clock();

    // print execution time
    printf("Execution Time: %f milliseconds\n", (double)(end - start) * 1000.0 / CLOCKS_PER_SEC);

    // cleanup
    node_release(node);

    return 0;
}
