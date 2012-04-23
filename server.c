#include <stdio.h>
#include "actor/actor.h"
#include "actor/distributer.h"

void main_process(actor_process_t self) {
    // listen on port 5000
    actor_process_id_t listener = actor_distributer_listen(self->node, 5000);

    while (true) {
        // receive message
        actor_message_t message = actor_message_receive(self, 10.0);

        // check timeout
        if (message == NULL) {
            break;
        }

        // print message
        printf("%s\n", (char*)message->data);

        actor_message_release(message);
    }
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
