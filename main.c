#include <stdio.h>
#include "actor/actor.h"
#include "actor/distributer.h"

void main_process(actor_process_t self) {
    // 
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
