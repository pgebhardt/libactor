#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "actor/actor.h"

actor_error_t client_handler(actor_process_t self, actor_node_id_t client_nid,
    actor_process_id_t client_pid, actor_process_id_t parent) {
    // error
    actor_error_t error = ACTOR_SUCCESS;

    // message
    actor_message_t message = NULL;

    // main loop
    while (true) {
        // receive message
        error = actor_message_receive(self, &message, 10.0);

        // check success
        if (error == ACTOR_ERROR_TIMEOUT) {
            continue;
        }
        else if (error != ACTOR_SUCCESS) {
            return error;
        }

        // send message to client
        error = actor_message_send(self, client_nid, client_pid,
            message->data, message->size);

        // check success
        if (error != ACTOR_SUCCESS) {
            // release message
            actor_message_release(message);

            return error;
        }

        // send message to parent
        actor_message_send(self, self->node->nid, parent,
            message->data, message->size);

        // release message
        actor_message_release(message);
    }

    return ACTOR_SUCCESS;
}

actor_error_t main_process(actor_process_t main) {
    // error
    actor_error_t error = ACTOR_SUCCESS;

    // message
    actor_message_t message = NULL;

    // client handler
    actor_process_id_t client_handler_pid = ACTOR_INVALID_ID;

    // listening loop
    while (true) {
        // wait on client connection
        actor_node_id_t client_nid = ACTOR_INVALID_ID;
        error = actor_node_listen(main->node, &client_nid, 3000);

        // check success
        if (error != ACTOR_SUCCESS) {
            printf("%d.%d error: %d\n", main->node->nid, main->pid, error);
            return error;
        }

        // get client pid
        error = actor_message_receive(main, &message, 10.0);

        // check success
        if (error != ACTOR_SUCCESS) {
            return error;
        }

        // client pid
        actor_process_id_t client_pid = *(actor_process_id_t*)message->data;
        printf("connected to %d.%d!\n", client_nid, client_pid);

        // start client handler
        actor_process_spawn(main->node, &client_handler_pid,
            ^actor_error_t(actor_process_t self) {
                return client_handler(self, client_nid, client_pid,
                    client_handler_pid);
            });

        // send client handler
        actor_message_send(main, client_nid, 0, &client_handler_pid,
            sizeof(actor_process_id_t));

        // release message
        actor_message_release(message);
    }

    return ACTOR_SUCCESS;
}

int main(int argc, char* argv[]) {
    // create node
    actor_node_t node = NULL;
    if (actor_node_create(&node, 0, 1000) != ACTOR_SUCCESS) {
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
