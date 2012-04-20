#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "message.h"
#include "node.h"
#include "process.h"

void main_process(actor_process_t main) {
    int size = 1000;

    // process function
    actor_process_function_t function = ^(actor_process_t self) {
            // wait for incomming message
            actor_message_t message = actor_message_receive(self, 1e4);

            // check for timeout
            if (message == NULL) {
                printf("%d timeout!\n", self->pid);
                return;
            }

            // print message
            printf("%d, ", self->pid);

            // release message
            actor_message_release(message);
        };

    // start dummy process
    for (int i = 0; i < size; i++) {
        actor_process_spawn(main->node, function);
    }

    sleep(2);

    // send messages
    for (int i = 0; i < size; i++) {
        // send message
        actor_message_send(main, main->pid + i + 1,
            actor_message_create("Hallo", 6));
    }

    sleep(2);
}

int main(int argc, char* argv[]) {
    // create node
    actor_node_t node = actor_node_create(0, 2000);

    // start main process
    actor_main_process(node, ^(actor_process_t self) {
                main_process(self);
            });

    // release node
    actor_node_release(node);

    return 0;
}
