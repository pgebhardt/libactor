#include <stdio.h>
#include "actor/actor.h"

void main_process(actor_process_t self) {
    // connect to server
    actor_node_id_t server = actor_node_connect(self->node, "127.0.0.1", 3000);
    printf("%d.%d: connected to %d!\n", self->node->nid, self->pid, server);

    // check success
    if (server == -1) {
        printf("%d.%d: Could not connect!\n", self->node->nid, self->pid);
        return;
    }

    // send ping message
    actor_message_t ping = actor_message_send(self, server, 0,
        "Ping!", 6);

    // check success
    if (ping == NULL) {
        printf("%d.%d: Could not send ping!\n", self->node->nid, self->pid);
        return;
    }
    else {
        printf("%d.%d: Ping send!\n", self->node->nid, self->pid);
    }

    // receive pong
    actor_message_t pong = actor_message_receive(self, 5.0);

    // check success
    if (pong == NULL) {
        printf("%d.%d: Did not receive pong!\n", self->node->nid, self->pid);
        return;
    }
    else {
        printf("%d.%d: Pong!\n", self->node->nid, self->pid);
        actor_message_release(pong);
    }
}

int main(int argc, char* argv[]) {
    // create node
    actor_node_t node = actor_node_create(1, 2000);

    // start main process
    actor_process_spawn(node, ^(actor_process_t self) {
                main_process(self);
            });

    // release node
    actor_node_release(node);

    return 0;
}
