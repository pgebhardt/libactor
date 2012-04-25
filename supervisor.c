#include <stdio.h>
#include "actor/actor.h"

actor_error_t main_process(actor_process_t main) {
    // connect to processes
    actor_node_id_t node = ACTOR_INVALID_ID;
    if (actor_node_connect(main->node, &node, "127.0.0.1", 3000) != ACTOR_SUCCESS) {
        return ACTOR_ERROR;
    }

    // start supervisor
    actor_process_id_t supervisor = ACTOR_INVALID_ID;
    actor_process_spawn(main->node, &supervisor, ^actor_error_t(actor_process_t self) {
            // loop
            while (true) {
                // receive error message
                actor_message_t message = NULL;
                if (actor_message_receive(self, &message, 10.0) != ACTOR_SUCCESS) {
                    printf("Supervisor: No process left!\n");

                    break;
                }

                // get error message
                actor_process_error_message_t error_message =
                    (actor_process_error_message_t)message->data;

                // print result
                printf("Supervisor: %d.%d died with %d!\n", error_message->nid,
                    error_message->pid, error_message->error);

                // release message
                actor_message_release(message);
            }

            return ACTOR_SUCCESS;
        });

    // send supervisor pid
    actor_message_send(main, node, 0, &supervisor, sizeof(actor_process_id_t));

    return ACTOR_SUCCESS;
}

int main(int argc, char* argv[]) {
    // create node
    actor_node_t node = NULL;
    if (actor_node_create(&node, 0, 100) != ACTOR_SUCCESS) {
        return EXIT_FAILURE;
    }

    // spawn main process
    actor_process_spawn(node, NULL, ^actor_error_t(actor_process_t main) {
            return main_process(main);
        });

    // release node
    actor_node_release(node);

    return EXIT_SUCCESS;
}

