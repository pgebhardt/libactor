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

    // connect to server
    actor_node_id_t server = ACTOR_INVALID_ID;
    error = actor_node_connect(self->node, &server, "127.0.0.1", 3000, "bla");

    // check success
    if (error != ACTOR_SUCCESS) {
        return error;
    }

    // main loop
    while (true) {
        // let client start pong process
        error = actor_send(self, server, 0, ACTOR_TYPE_CUSTOM, "Start", 6);

        // check success
        if (error != ACTOR_SUCCESS) {
            return error;
        }

        // receive pong id
        actor_message_t message = NULL;
        error = actor_receive(self, &message, 10.0);

        // check success
        if (error != ACTOR_SUCCESS) {
            return error;
        }

        // check message type
        if (message->type != ACTOR_TYPE_UINT) {
            return ACTOR_ERROR;
        }

        // get pong id
        actor_process_id_t pong = *(actor_process_id_t*)message->data;

        // release message
        actor_message_release(&message);

        // start ping process
        actor_spawn(self->node, NULL, ^actor_error_t(actor_process_t ping) {
            // link to main process
            actor_process_link(ping, self->nid, self->pid);

            // send ping to pong process
            actor_send(ping, server, pong, ACTOR_TYPE_UINT, &ping->pid,
                sizeof(actor_process_id_t));

            // print
            printf("%d.%d sent ping to %d.%d\n", ping->nid, ping->pid,
                server, pong);

            // receive pong message
            actor_message_t pong_message = NULL;
            if (actor_receive(ping, &pong_message, 10.0) != ACTOR_SUCCESS) {
                return ACTOR_ERROR;
            }

            // check type
            if (pong_message->type != ACTOR_TYPE_UINT) {
                // release message
                actor_message_release(&pong_message);

                return ACTOR_ERROR;
            }

            // print message
            printf("%d.%d received pong from %d.%d\n", ping->nid, ping->pid,
                server, *(actor_process_id_t*)pong_message->data);

            // release message
            actor_message_release(&pong_message);

            return ACTOR_SUCCESS;
        });

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

        // wait
        actor_process_sleep(self, 2.0);
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
    actor_spawn(node, NULL, ^actor_error_t(actor_process_t self) {
        return main_process(self);
    });

    // wait for processes to complete
    actor_node_wait_for_processes(node, 30.0);

    // release node
    actor_node_release(&node);

    return EXIT_SUCCESS;
}
