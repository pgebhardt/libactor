#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "message.h"
#include "node.h"
#include "process.h"

void main_process(process_process* main) {
    int size = 10;

    // process function
    process_process_function function = ^(process_process* self) {
            printf("Ich bin Prozess %d!\n", self->pid);
            process_message_send(self, main->pid,
                message_message_create("Hallo", 6));
        };

    // start dummy process
    for (int i = 0; i < size; i++) {
        node_process_spawn(main->node, function);
    }

    // receive messages
    for (int i = 0; i < size; i++) {
        // receive message
        message_message* message = process_message_receive(main, 10.0);

        // output message
        printf("%s\n", (char*)message->message_data);

        // release message
        message_message_release(message);
    }
}

int main(int argc, char* argv[]) {
    // create node
    node_node* node = node_create(0, 2000);

    // start main process
    node_main_process(node, ^(process_process* self) {
                main_process(self);
            });

    // release node
    node_release(node);

    return 0;
}
