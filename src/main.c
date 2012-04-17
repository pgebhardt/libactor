#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "node.h"
#include "process.h"
#include "message.h"

int main(int argc, char* argv[]) {
    // create node
    node_node* node = node_create(0);
    // spawn dummy process
    process_process* process = process_spawn(node, ^(process_process* self){
            // print some stuff
            printf("Hallo du da, was geht?\n");

            // receive message
            message_message* message = message_queue_get(self->queue, 4.0f);

            // output message
            printf("Message received: %f\n", *(double*)message->message_data);

            // cleanup message
            message_message_cleanup(message);
        });

    // output pid
    printf("Pid: %d\n", process->pid);

    // create message
    double data = 20.0;
    message_message* message = message_message_create(&data, sizeof(double));

    // sleep a bit
    sleep(2);

    // send message
    message_queue_put(process->queue, message);

    // sleep a bit
    sleep(1);

    // cleanup
    process_cleanup(process);
    node_cleanup(node);

    return 0;
}
