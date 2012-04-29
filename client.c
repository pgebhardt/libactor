#include <stdio.h>
#include "actor.h"

actor_error_t main_process(actor_process_t self) {
    // error
    actor_error_t error = ACTOR_SUCCESS;

    // connect to server
    actor_node_id_t server = ACTOR_INVALID_ID;
    error = actor_node_connect(self->node, &server, "127.0.0.1", 3000);

    // check success
    if (error != ACTOR_SUCCESS) {
        return error;
    }

    // main loop
    while (true) {
        // receive start message
        actor_message_t message = NULL;
        error = actor_message_receive(self, &message, 10.0);

        // check success
        if (error != ACTOR_SUCCESS) {
            return error;
        }

        // check message type
        if (message->type != ACTOR_TYPE_CUSTOM) {
            // release message
            actor_message_release(message);

            return ACTOR_ERROR;
        }

        // start pong process
        actor_process_id_t pong_process = ACTOR_INVALID_ID;
        actor_process_spawn(self->node, &pong_process,
            ^actor_error_t(actor_process_t pong) {
                // link to main process
                actor_process_link(pong, self->nid, self->pid);

                // receive ping message
                actor_message_t ping_message = NULL;
                if (actor_message_receive(pong, &ping_message, 10.0)
                    != ACTOR_SUCCESS) {
                    return ACTOR_ERROR;
                }

                // check type
                if (ping_message->type != ACTOR_TYPE_UINT) {
                    // release message
                    actor_message_release(ping_message);

                    return ACTOR_ERROR;
                }

                // print
                printf("%d.%d received ping from %d.%d\n", pong->nid, pong->pid,
                    server, *(actor_process_id_t*)ping_message->data);

                // send pong
                actor_message_send(pong, server,
                    *(actor_process_id_t*)ping_message->data,
                    ACTOR_TYPE_UINT, &pong->pid, sizeof(actor_process_id_t));

                // print
                printf("%d.%d sent pong to %d.%d\n", pong->nid, pong->pid,
                    server, *(actor_process_id_t*)ping_message->data);

                // release message
                actor_message_release(ping_message);

                return ACTOR_SUCCESS;
            });

        // send pong id
        actor_message_send(self, server, 0, ACTOR_TYPE_UINT,
            &pong_process, sizeof(actor_process_id_t));

        // receive quit message
        error = actor_message_receive(self, &message, 10.0);

        // check success
        if (error != ACTOR_SUCCESS) {
            return error;
        }

        // get error message
        actor_process_error_message_t error_message =
            (actor_process_error_message_t)message->data;

        // check error
        if (error_message->error != ACTOR_SUCCESS) {
            // release message
            actor_message_release(message);

            return ACTOR_ERROR;
        }
    }

    return ACTOR_SUCCESS;
}

int main(int argc, char* argv[]) {
    // create node
    actor_node_t node = NULL;
    if (actor_node_create(&node, 1, 100) != ACTOR_SUCCESS) {
        return EXIT_FAILURE;
    }

    // start main process
    actor_process_spawn(node, NULL, ^actor_error_t(actor_process_t self) {
        return main_process(self);
    });

    // release node
    actor_node_release(node);

    return EXIT_SUCCESS;
}
