// libactor
//
// Implementation of an erlang style actor model using libdispatch
// Copyright (C) 2012  Patrik Gebhardt
// Contact: patrik.gebhardt@rub.de
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <stdio.h>
#include "../include/actor.h"

actor_error_t main_process(actor_process_t self) {
    // error
    actor_error_t error = ACTOR_SUCCESS;

    // wait for client connection
    actor_node_id_t client = ACTOR_INVALID_ID;
    error = actor_node_listen(self->node, &client, 3000, "bla");

    // check success
    if (error != ACTOR_SUCCESS) {
        return error;
    }

    // main loop
    while (true) {
        // receive start message
        actor_message_t message = NULL;
        error = actor_receive(self, &message, 10.0);

        // check success
        if (error != ACTOR_SUCCESS) {
            return error;
        }

        // check message type
        if (message->type != ACTOR_TYPE_CUSTOM) {
            // release message
            actor_message_release(&message);

            return ACTOR_ERROR;
        }

        // start pong process
        actor_process_id_t pong_process = ACTOR_INVALID_ID;
        actor_spawn(self->node, &pong_process,
            ^actor_error_t(actor_process_t pong) {
                // link to main process
                actor_process_link(pong, self->nid, self->pid);

                // receive ping message
                actor_message_t ping_message = NULL;
                if (actor_receive(pong, &ping_message, 10.0)
                    != ACTOR_SUCCESS) {
                    return ACTOR_ERROR;
                }

                // check type
                if (ping_message->type != ACTOR_TYPE_UINT) {
                    // release message
                    actor_message_release(&ping_message);

                    return ACTOR_ERROR;
                }

                // print
                printf("%d.%d received ping from %d.%d\n", pong->nid, pong->pid,
                    client, *(actor_process_id_t*)ping_message->data);

                // send pong
                actor_send(pong, client,
                    *(actor_process_id_t*)ping_message->data,
                    ACTOR_TYPE_UINT, &pong->pid, sizeof(actor_process_id_t));

                // print
                printf("%d.%d sent pong to %d.%d\n", pong->nid, pong->pid,
                    client, *(actor_process_id_t*)ping_message->data);

                // release message
                actor_message_release(&ping_message);

                return ACTOR_SUCCESS;
            });

        // send pong id
        actor_send(self, client, 0, ACTOR_TYPE_UINT,
            &pong_process, sizeof(actor_process_id_t));

        // receive quit message
        error = actor_receive(self, &message, 10.0);

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
            actor_message_release(&message);

            return ACTOR_ERROR;
        }

        // release message
        actor_message_release(&message);
    }

    return ACTOR_SUCCESS;
}

int main(int argc, char* argv[]) {
    // create node
    actor_node_t node = NULL;
    if (actor_node_create(&node, 0, 100) != ACTOR_SUCCESS) {
        return EXIT_FAILURE;
    }

    // start main process
    actor_spawn(node, NULL, ^actor_error_t(actor_process_t self) {
        return main_process(self);
    });

    // release node
    actor_node_release(&node);

    return EXIT_SUCCESS;
}
