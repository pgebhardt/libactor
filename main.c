#include <stdio.h>
#include "actor/actor.h"

actor_error_t main_process(actor_process_t main) {
    // start ping process
    actor_process_id_t pid = ACTOR_INVALID_ID;
    actor_process_spawn(main->node, &pid, ^actor_error_t(actor_process_t self) {
            // send ping message to main
            if (actor_message_send(self, main->node->nid, main->pid,
                    "Ping!", 6) != ACTOR_SUCCESS) {
                printf("%d.%d: Could not send ping!\n", self->node->nid, self->pid);
                return ACTOR_FAILURE;
            }

            // print ping send
            printf("%d.%d: Ping sent!\n", self->node->nid, self->pid);

            // receive pong
            actor_message_t pong = NULL;
            if (actor_message_receive(self, &pong, 10.0) != ACTOR_SUCCESS) {
                printf("%d.%d: Did not receive pong!\n", self->node->nid, self->pid);
                return ACTOR_FAILURE;
            }

            // print message
            printf("%d.%d: Received %s\n", self->node->nid, self->pid,
                (char*)pong->data);

            // release message
            actor_message_release(pong);

            return ACTOR_SUCCESS;
        });

    // receive ping
    actor_message_t ping = NULL;
    if (actor_message_receive(main, &ping, 10.0) != ACTOR_SUCCESS) {
        printf("%d.%d: Did not receive ping!\n", main->node->nid, main->pid);
        return ACTOR_FAILURE;
    }

    // print message
    printf("%d.%d: Received %s\n", main->node->nid, main->pid, (char*)ping->data);

    // release message
    actor_message_release(ping);

    // send pong
    if (actor_message_send(main, main->node->nid, pid, "Pong!", 6) != ACTOR_SUCCESS) {
        printf("%d.%d: Could not send pong!\n", main->node->nid, main->pid);
        return ACTOR_FAILURE;
    }

    // print pong send
    printf("%d.%d: Pong sent!\n", main->node->nid, main->pid);

    return ACTOR_SUCCESS;
}

int main(int argc, char* argv[]) {
    // create node
    actor_node_t node = NULL;
    if (actor_node_create(&node, 0, 1000) != ACTOR_SUCCESS) {
        return EXIT_FAILURE;
    }

    // spawn main process
    actor_process_spawn(node, NULL, ^actor_error_t(actor_process_t main) {
            // start main process supervisor
            actor_process_id_t supervisor;
            actor_process_spawn(node, &supervisor,
                ^actor_error_t(actor_process_t self) {
                    // receive error message
                    actor_message_t message = NULL;
                    if (actor_message_receive(self, &message, 10.0) != ACTOR_SUCCESS) {
                        return ACTOR_FAILURE;
                    }

                    // cast to error message
                    actor_process_error_message_t error_message =
                        (actor_process_error_message_t)message->data;

                    // print error
                    printf("%d.%d died with %d!\n", error_message->nid,
                        error_message->pid, error_message->error);

                    // release message
                    actor_message_release(message);

                    return ACTOR_SUCCESS;
                });

            // link to supervisor
            actor_process_link(main, node->nid, supervisor);

            return main_process(main);
        });

    // release node
    actor_node_release(node);

    return EXIT_SUCCESS;
}
