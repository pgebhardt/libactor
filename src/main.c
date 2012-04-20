#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "message.h"
#include "node.h"
#include "process.h"

void main_process(actor_process_t main) {
    int size = 100;

    // process function
    actor_process_function_t function = ^(actor_process_t self) {
            printf("Ich bin Prozess %d!\n", self->pid);
            actor_message_send(self, main->pid,
                actor_message_create("Hallo", 6));
        };

    // start dummy process
    for (int i = 0; i < size; i++) {
        actor_process_spawn(main->node, function);
    }

    // receive messages
    for (int i = 0; i < size; i++) {
        // receive message
        actor_message_t message = actor_message_receive(main, 10.0);

        // output message
        printf("%s\n", (char*)message->message_data);

        // release message
        actor_message_release(message);
    }
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
