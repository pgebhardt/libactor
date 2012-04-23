#include <stdio.h>
#include "actor/actor.h"
#include "actor/distributer.h"

void main_process(actor_process_t self) {
    // connect to port 5000
    actor_process_id_t connector = actor_distributer_connect_to_node(self->node,
        "127.0.0.1", 5000);

    // send messages
    actor_distributer_message_t distributer_message = malloc(sizeof(actor_distributer_message_struct));
    for (int i = 0; i < 10; i++) {
        // create message
        actor_message_t message = actor_message_create("Hallo", 6);

        // set distributer_message
        distributer_message->dest_id = 0;
        distributer_message->message = message;

        // send message
        actor_message_send(self, connector, distributer_message,
            sizeof(actor_distributer_message_struct));
    }

    if (connector != -1) {
        printf("Gesendet!\n");
    }

    // free memory
    free(distributer_message);
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
