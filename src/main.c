#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "message.h"
#include "node.h"
#include "process.h"

int main(int argc, char* argv[]) {
    // create node
    node_node* node = node_create(0, 100);
    // spawn dummy process
    process_process* process1 = node_process_spawn(node, ^(process_process* self) {
            // print some stuff
            printf("Ich bin prozess %d\n", self->pid);

            // receive message
            message_message* message = process_message_receive(self, 4.0f);

            if (message != NULL) {
                // output message
                printf("Message received: %p, %p\n", message, message->message_data);

                // cleanup message
                message_message_cleanup(message);
            }
        });

    // spawn dummy process
    process_process* process2 = node_process_spawn(node, ^(process_process* self) {
            // print some stuff
            printf("Ich bin prozess %d\n", self->pid);

            // receive message
            message_message* message = process_message_receive(self, 4.0f);

            if (message != NULL) {
                // output message
                printf("Message received: %p, %p\n", message, message->message_data);

                // cleanup message
                message_message_cleanup(message);
            }
        });

    // create message
    double data = 20.0;
    printf("Data address: %p\n", &data);
    message_message* message1 = message_message_create(&data, sizeof(double));
    data = 40.0;
    message_message* message2 = message_message_create(&data, sizeof(double));

    // sleep a bit
    sleep(2);

    // send message
    process_message_send(process1, process1->pid, message1);
    process_message_send(process2, process2->pid, message2);

    // sleep a bit
    sleep(1);

    // cleanup
    node_cleanup(node);

    return 0;
}
