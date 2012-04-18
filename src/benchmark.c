#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "node.h"
#include "message.h"
#include "process.h"

void main_process(process_process* self) {
    // process function
    process_process_function function = ^(process_process* s) {
        message_message* message = NULL;

        while (true) {
            // get message
            message = process_message_receive(s, 0.1);

            if (message == NULL) {
                break;
            }

            // print message
            printf("Process %d, %s\n", s->pid, (char*)message->message_data);

            // answer
            process_message_send(s, self->pid, message_message_create("Pong", 5));

            // release message
            message_message_cleanup(message);
        }
    };

    // start 4 processes
    process_process* pid1 = node_process_spawn(self->node, function);
    process_process* pid2 = node_process_spawn(self->node, function);
    process_process* pid3 = node_process_spawn(self->node, function);
    process_process* pid4 = node_process_spawn(self->node, function);

    // send message
    printf("Pre send\n");
    process_message_send(self, pid1->pid, message_message_create("Ping1", 6));
    process_message_send(self, pid2->pid, message_message_create("Ping2", 6));
    process_message_send(self, pid3->pid, message_message_create("Ping3", 6));
    process_message_send(self, pid4->pid, message_message_create("Ping4", 6));
    process_message_send(self, pid1->pid, message_message_create("Ping5", 6));
    process_message_send(self, pid2->pid, message_message_create("Ping6", 6));
    process_message_send(self, pid3->pid, message_message_create("Ping7", 6));
    process_message_send(self, pid4->pid, message_message_create("Ping8", 6));
    printf("Post send\n");

    message_message* response = NULL;

    while (true) {
        // wait for response
        response = process_message_receive(self, 0.1);

        if (response == NULL) {
            break;
        }

        // print message
        printf("%s\n", (char*)response->message_data);

        // free message
        message_message_cleanup(response);
    }
}

int main(int argc, char* argv[]) {
    // create node
    node_node* node = node_create(0, 10000);

    // time variables
    clock_t start, end;

    // get start time
    start = clock();

    // start main process
    node_main_process(node, ^(process_process* self) {
            main_process(self);
        });

    // get end time
    end = clock();

    // print execution time
    printf("Execution Time: %f milliseconds\n", (double)(end - start) * 1000.0 / CLOCKS_PER_SEC);

    // cleanup
    node_cleanup(node);

    return 0;
}
