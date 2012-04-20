#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "message.h"
#include "node.h"
#include "process.h"

void main_process(actor_process_t self) {
    printf("Begin!\n");

    // wait
    actor_process_sleep(self, 2.0);

    printf("After wait 1!\n");

    actor_process_sleep(self, 2.0);

    printf("After wait 2!\n");

    actor_process_spawn(self->node, ^(actor_process_t s) {
            // sleep a bit
            actor_process_sleep(s, 2.0);
            printf("Ende!\n");
        });
}

int main(int argc, char* argv[]) {
    // create node
    actor_node_t node = actor_node_create(0, 2000);

    // start main process
    actor_process_spawn(node, ^(actor_process_t self) {
                main_process(self);
            });

    // release node
    actor_node_release(node);

    return 0;
}
