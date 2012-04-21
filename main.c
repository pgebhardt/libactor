#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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
                actor_message_send(s, self->pid,
                    actor_message_create("Hallo", 6));
            });

    // send message
    actor_message_send(self, pid, actor_message_create("Jo", 3));

    // receive message
    actor_message_t response = actor_message_receive(self, 10.0);

    // check success
    if (response == NULL) {
        return;
    }

    // output message
    printf("%d: %s\n", self->pid, (char*)response->data);

    // release message
    actor_message_release(response);
}

int main(int argc, char* argv[]) {
    // create node
    actor_node_t node = actor_node_create(0, 2000);

    printf("%p\n", node);

    // start main process
    actor_process_spawn(node, ^(actor_process_t self) {
                main_process(self);
            });

    sleep(10);

    // release node
    actor_node_release(node);

    return 0;
}
