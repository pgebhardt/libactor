#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "message.h"
#include "node.h"
#include "process.h"

void main_process(process_process* main) {
    // process function
    process_process_function function = ^(process_process* self) {
            // wait a second
            sleep(1);

            // create answer
            char answer[100];
            sprintf(answer, "Hi, i'm process %d!", self->pid);

            // send message to parent
            process_message_send(self, main->pid,
                message_message_create(answer, strlen(answer) + 1));
        };

    // start processes
    for (int i = 0; i < 100; i++) {
        node_process_spawn(main->node, function);
    }

    // gather messages
    while (true) {
        // receive message
        message_message* message = process_message_receive(main, 2.0);

        if (message == NULL) {
            return;
        }

        // output message
        printf("%s\n", (char*)message->message_data);

        // release message
        message_message_release(message);
    }
}

int main(int argc, char* argv[]) {
    // create node
    node_node* node = node_create(0, 1000);

    // start main process
    node_main_process(node, ^(process_process* self) {
                main_process(self);
            });

    // release node
    node_release(node);

    return 0;
}
