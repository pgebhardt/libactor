#include <stdio.h>
#include "actor/actor.h"

actor_error_t main_process(actor_process_t main) {
    // wait for supervisor
    actor_node_id_t supervisor_nid = ACTOR_INVALID_ID;
    if (actor_node_listen(main->node, &supervisor_nid, 3000) != ACTOR_SUCCESS) {
        return ACTOR_ERROR;
    }

    // get supervisor pid
    actor_message_t supervisor_message;
    if (actor_message_receive(main, &supervisor_message, 10.0) != ACTOR_SUCCESS) {
        return ACTOR_ERROR;
    }

    // get supervisor pid
    actor_process_id_t supervisor_pid = *(actor_process_id_t*)supervisor_message->data;

    // release message
    actor_message_release(supervisor_message);

    // pong process function
    actor_error_t (^pong_function)(actor_process_t, actor_process_id_t) =
        ^actor_error_t(actor_process_t self, actor_process_id_t ping) {
            // receive message
            actor_message_t message = NULL;
            if (actor_message_receive(self, &message, 2.0) != ACTOR_SUCCESS) {
                return ACTOR_ERROR_TIMEOUT;
            }

            // print ping
            printf("%d.%d: received %s\n", self->node->nid, self->pid,
                (char*)message->data);

            // release message
            actor_message_release(message);

            // send pong
            if (actor_message_send(self, self->node->nid, ping, "Pong!", 6)
                != ACTOR_SUCCESS) {
                return ACTOR_ERROR;
            }

            return ACTOR_SUCCESS;
        };

    // ping process function
    actor_process_function_t ping_function = ^actor_error_t(actor_process_t self) {
            // spawn pong process
            actor_process_id_t pong = ACTOR_INVALID_ID;
            actor_process_spawn(self->node, &pong, ^actor_error_t(actor_process_t s) {
                    // link to ping supervisor
                    actor_process_link(s, self->supervisor_nid, self->supervisor_pid);

                    // start pong function
                    return pong_function(s, self->pid);
                });

            // send ping to pong
            if (actor_message_send(self, self->node->nid, pong, "Ping!", 6)
                != ACTOR_SUCCESS) {
                return ACTOR_ERROR;
            }

            // receive message
            actor_message_t message = NULL;
            if (actor_message_receive(self, &message, 2.0) != ACTOR_SUCCESS) {
                return ACTOR_ERROR_TIMEOUT;
            }

            // print pong
            printf("%d.%d: received %s\n", self->node->nid, self->pid,
                (char*)message->data);

            // release message
            actor_message_release(message);

            return ACTOR_SUCCESS;
        };

    // start ping pong several times
    for (int i = 0; i < 10; i++) {
        // start ping process
        actor_process_spawn(main->node, NULL, ^actor_error_t(actor_process_t self) {
                // link to supervisor
                actor_process_link(self, supervisor_nid, supervisor_pid);

                // start ping function
                return ping_function(self);
            });

        // sleep
        actor_process_sleep(main, 2.0);
    }

    return ACTOR_SUCCESS;
}

int main(int argc, char* argv[]) {
    // create node
    actor_node_t node = NULL;
    if (actor_node_create(&node, 1, 100) != ACTOR_SUCCESS) {
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
