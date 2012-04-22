#include <stdio.h>
#include "actor/actor.h"

void main_process(actor_process_t self) {
    // start second process
    actor_process_id_t pid = actor_process_spawn(self->node, ^(actor_process_t s) {
                // receive message
                actor_message_t request = actor_message_receive(s, 10.0);

                // check success
                if (request == NULL) {
                    return;
                }

                // output data
                printf("%d: %s\n", s->pid, (char*)request->data);

                // release message
                actor_message_release(request);

                // send message
                actor_message_send(s, self->pid, "Hallo", 6);
            });

    // send message
    actor_message_send(self, pid, "Jo1", 4);
    actor_message_send(self, self->pid, "Jo2", 4);

    // receive message
    actor_message_t message = NULL;
    do {
        // get message
        message = actor_message_receive(self, 1.0);

        // print message
        if (message != NULL) {
            printf("%d: %s\n", self->pid, (char*)message->data);
        }

        // release message
        actor_message_release(message);
    } while (message != NULL);
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
