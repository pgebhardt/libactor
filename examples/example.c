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
#include <actor/actor.h>

actor_error_t main_process(actor_process_t main) {
    // ping funktion
    actor_error_t (^ping_function)(actor_process_t) = ^actor_error_t(actor_process_t ping) {
        // start pong process
        actor_process_id_t pong_id = ACTOR_INVALID_ID;
        actor_spawn(ping->node, &pong_id, ^actor_error_t(actor_process_t pong) {
            // receive ping message
            actor_message_t message = NULL;
            if (actor_receive(pong, &message, 10.0) != ACTOR_SUCCESS) {
                return ACTOR_ERROR;
            }

            // check message type
            if (message->type != ACTOR_TYPE_CHAR) {
                return ACTOR_ERROR;
            }

            // print message
            printf("%d.%d received %s\n", pong->nid, pong->pid, (char*)message->data);

            // release message
            actor_message_release(&message);

            // send pong
            actor_send(pong, ping->nid, ping->pid, ACTOR_TYPE_CHAR, "Pong!", 6);

            return ACTOR_SUCCESS;
        });

        // send ping message
        actor_send(ping, ping->nid, pong_id, ACTOR_TYPE_CHAR, "Ping!", 6);

        // receive message
        actor_message_t message = NULL;
        if (actor_receive(ping, &message, 10.0) != ACTOR_SUCCESS) {
            return ACTOR_ERROR;
        }

        // check message type
        if (message->type != ACTOR_TYPE_CHAR) {
            return ACTOR_ERROR;
        }

        // print message
        printf("%d.%d received %s\n", ping->nid, ping->pid, (char*)message->data);

        // release message
        actor_message_release(&message);

       return ACTOR_SUCCESS;
    };

    // main loop
    while (true) {
        // start ping process
        actor_spawn(main->node, NULL, ^actor_error_t(actor_process_t self) {
            // link to main
            actor_process_link(self, main->nid, main->pid);

            // start ping process
            return ping_function(self);
        });

        // receive error
        actor_message_t message = NULL;
        if (actor_receive(main, &message, 10.0) != ACTOR_SUCCESS) {
            return ACTOR_ERROR;
        }

        // check message type
        if (message->type != ACTOR_TYPE_ERROR_MESSAGE) {
            return ACTOR_ERROR;
        }

        // release message
        actor_message_release(&message);

        // slepp a bit
        actor_process_sleep(main, 1.0);
    }

    return ACTOR_SUCCESS;
}

int main(int argc, char* argv[]) {
    // create node
    actor_node_t node = NULL;
    if (actor_node_create(&node, 0, 10) != ACTOR_SUCCESS) {
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
