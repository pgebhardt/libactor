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

actor_error_t ping_function(actor_process_t self) {
    // error
    actor_error_t error = ACTOR_SUCCESS;

    // get pong id
    actor_message_t message = NULL;
    error = actor_receive(self, &message, 10.0);

    // check success
    if (error != ACTOR_SUCCESS) {
        return error;
    }

    // check message type
    if (message->type != ACTOR_TYPE_PROCESSID) {
        // cleanup
        actor_message_release(&message);

        return ACTOR_ERROR;
    }

    // cast pong id
    actor_process_id_t pong = *(actor_process_id_t*)message->data;

    // release message
    actor_message_release(&message);

    // send ping to pong
    error = actor_send(self, self->nid, pong, ACTOR_TYPE_CHAR, "Ping!", 6);

    // check success
    if (error != ACTOR_SUCCESS) {
        return error;
    }

    printf("%d.%d: Ping sent to %d.%d!\n", self->nid, self->pid, self->nid,
        pong);

    // receive pong
    error = actor_receive(self, &message, 10.0);

    // check success
    if (error != ACTOR_SUCCESS) {
        return error;
    }

    // check message type
    if (message->type != ACTOR_TYPE_CHAR) {
        // cleanup
        actor_message_release(&message);

        return ACTOR_ERROR;
    }

    // print message
    printf("%d.%d: received %s\n", self->nid, self->pid, (char*)message->data);

    // release message
    actor_message_release(&message);

    return ACTOR_SUCCESS;
}

actor_error_t pong_function(actor_process_t self) {
    // error
    actor_error_t error = ACTOR_SUCCESS;

    // get ping id
    actor_message_t message = NULL;
    error = actor_receive(self, &message, 10.0);

    // check success
    if (error != ACTOR_SUCCESS) {
        return error;
    }

    // check message type
    if (message->type != ACTOR_TYPE_PROCESSID) {
        // cleanup
        actor_message_release(&message);

        return ACTOR_ERROR;
    }

    // cast ping id
    actor_process_id_t ping = *(actor_process_id_t*)message->data;

    // release message
    actor_message_release(&message);

    // receive ping
    error = actor_receive(self, &message, 10.0);

    // check success
    if (error != ACTOR_SUCCESS) {
        return error;
    }

    // check message type
    if (message->type != ACTOR_TYPE_CHAR) {
        // cleanup
        actor_message_release(&message);

        return ACTOR_ERROR;
    }

    // print message
    printf("%d.%d: received %s\n", self->nid, self->pid, (char*)message->data);

    // release message
    actor_message_release(&message);

    // send pong to ping
    error = actor_send(self, self->nid, ping, ACTOR_TYPE_CHAR, "Pong!", 6);

    // check success
    if (error != ACTOR_SUCCESS) {
        return error;
    }

    printf("%d.%d: Pong sent to %d.%d!\n", self->nid, self->pid, self->nid,
        ping);

    return ACTOR_SUCCESS;
}

actor_error_t main_process(actor_process_t main) {
    // error
    actor_error_t error = ACTOR_SUCCESS;

    // main loop
    while (true) {
        // spawn ping process
        actor_process_id_t ping = ACTOR_INVALID_ID;
        error = actor_spawn(main->node, &ping, ^actor_error_t(actor_process_t self) {
            // link to main process
            actor_process_link(self, main->nid, main->pid);

            return ping_function(self);
        });

        // check success
        if (error != ACTOR_SUCCESS) {
            return error;
        }

        // spawn pong process
        actor_process_id_t pong = ACTOR_INVALID_ID;
        error = actor_spawn(main->node, &pong, ^actor_error_t(actor_process_t self) {
            // link to main process
            actor_process_link(self, main->nid, main->pid);

            return pong_function(self);
        });

        // check success
        if (error != ACTOR_SUCCESS) {
            return error;
        }

        // send process ids
        actor_send(main, main->nid, pong, ACTOR_TYPE_PROCESSID, &ping, sizeof(actor_process_id_t));
        actor_send(main, main->nid, ping, ACTOR_TYPE_PROCESSID, &pong, sizeof(actor_process_id_t));

        // get result of both processes
        for (int i = 0; i < 2; i++) {
            // message loop
            while (true) {
                // get error message
                actor_message_t message = NULL;
                error = actor_receive(main, &message, 2.0);

                // check success
                if (error != ACTOR_SUCCESS) {
                    return error;
                }

                // check message type
                if (message->type != ACTOR_TYPE_ERROR_MESSAGE) {
                    // cleanup
                    actor_message_release(&message);

                    continue;
                }

                // cast to error message
                actor_process_error_message_t error_message =
                    (actor_process_error_message_t)message->data;

                // check error
                if (error_message->error != ACTOR_SUCCESS) {
                    // cleanup
                    actor_message_release(&message);

                    return error_message->error;
                }

                // exit loop
                break;
            }
        }

        // wait a bit
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
            // call main process
            actor_error_t error = main_process(self);

            // print result
            printf("main process died with result: %s!\n", actor_error_string(error));

            return error;
        });

    // release node
    actor_node_release(&node);

    return EXIT_SUCCESS;
}
