#include <stdio.h>
#include "actor/actor.h"

void main_process(actor_process_t self) {
    // waiting for incomming connection
    actor_node_id_t client = actor_node_listen(self->node, 3000);
    printf("%d.%d: connected to %d!\n", self->node->nid, self->pid, client);

    // check success
    if (client == -1) {
        printf("%d.%d: Could not connect!\n", self->node->nid, self->pid);
        return;
    }

    // receive ping message
    actor_message_t ping = actor_message_receive(self, 5.0);

    // check success
    if (ping == NULL) {
        printf("%d.%d: Timeout!\n", self->node->nid, self->pid);
        return;
    }
    else {
        printf("%d.%d: Ping!\n", self->node->nid, self->pid);
        actor_message_release(ping);
    }

    // send pong
    actor_message_t pong = actor_message_send(self, client, 0, "Pong!", 6);

    // check success
    if (pong == NULL) {
        printf("%d.%d: Cannot send pong!\n", self->node->nid, self->pid);
    }
    else {
        printf("%d.%d: Pong sent!\n", self->node->nid, self->pid);
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
