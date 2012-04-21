#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "actor/actor.h"

void main_process(actor_process_t self) {
    // process id memory
    int processes = 1000;
    actor_process_id_t* pids = malloc(processes * sizeof(actor_process_id_t));

    // process function
    actor_process_function_t function = ^(actor_process_t s) {
        printf("I'm procces %d!\n", s->pid);
    };

    // create processes
    for (int i = 0; i < processes; i++) {
        actor_process_spawn(self->node, function);
    }

    // wait
    actor_process_sleep(self, 2.0);
}

int main(int argc, char* argv[]) {
    // create node
    actor_node_t node = actor_node_create(0, 10000);

    // time variables
    clock_t start, end;

    // get start time
    start = clock();

    // start main process
    actor_process_spawn(node, ^(actor_process_t self) {
            main_process(self);
        });

    // get end time
    end = clock();

    // print execution time
    printf("Execution Time: %f milliseconds\n", (double)(end - start) * 1000.0 / CLOCKS_PER_SEC);

    // cleanup
    actor_node_release(node);

    return 0;
}
