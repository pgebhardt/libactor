#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "actor/actor.h"

actor_error_t main_process(actor_process_t main) {
    // error
    actor_error_t error = ACTOR_SUCCESS;

    // connect to server
    actor_node_id_t server_nid = ACTOR_INVALID_ID;
    error = actor_node_connect(main->node, &server_nid, "127.0.0.1", 3000);

    // check success
    if (error != ACTOR_SUCCESS) {
        return error;
    }

    // start printer
    actor_process_id_t printer = ACTOR_INVALID_ID;
    actor_process_spawn(main->node, &printer,
        ^actor_error_t(actor_process_t self) {
            // message
            actor_message_t message = NULL;

            // main loop
            while (true) {
                // receive message
                if (actor_message_receive(self, &message, 10.0) != ACTOR_SUCCESS) {
                    continue;
                }

                // print message
                printf("%s\n", (char*)message->data);

                // release message
                actor_message_release(message);
            }

            return ACTOR_SUCCESS;
        });

    // send printer pid
    actor_message_send(main, server_nid, 0, &printer, sizeof(actor_process_id_t));

    // get client handler
    actor_message_t message = NULL;
    error = actor_message_receive(main, &message, 10.0);

    // check success
    if (error != ACTOR_SUCCESS) {
        return error;
    }

    // client handler pid
    actor_process_id_t client_handler = ACTOR_INVALID_ID;
    client_handler = *(actor_process_id_t*)message->data;

    // release message
    actor_message_release(message);

    // send message
    actor_message_send(main, server_nid, client_handler, "Hallo", 6);

    // read line and send to server
    char buffer[100];
    while (true) {
        // read line
        scanf("%s", buffer);

        // send to server
        if (actor_message_send(main, server_nid, client_handler, buffer,
            strlen(buffer) + 1) != ACTOR_SUCCESS) {
            return ACTOR_ERROR;
        }
    }

    return ACTOR_SUCCESS;
}

int main(int argc, char* argv[]) {
    // create node
    actor_node_t node = NULL;
    if (actor_node_create(&node, atoi(argv[1]), 1000) != ACTOR_SUCCESS) {
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
