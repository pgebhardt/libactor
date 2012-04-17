#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "process.h"
#include "message.h"

int main(int argc, char* argv[]) {
    // spawn dummy process
    process_process* process = process_spawn(^(process_process* self){
            // print some stuff
            printf("Hallo du da, was geht?\n");

            // receive message
            message_message* message = message_queue_get(self->queue, 2.0f);

            // output message
            printf("Message received: %p\n", message);

            free(message);
        });

    // output pid
    printf("Pid: %d\n", process->pid);

    // create message
    message_message* message = malloc(sizeof(message_message));

    // sleep a bit
    sleep(2);

    // send message
    message_queue_put(process->queue, message);

    // sleep a bit
    sleep(1);

    // cleanup
    process_cleanup(process);

    return 0;
}
