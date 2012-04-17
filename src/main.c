#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "process.h"
#include "message.h"

int main(int argc, char* argv[]) {
    // spawn dummy process
    process_process* process = process_spawn(^{
            // print some stuff
            printf("Hallo du da, was geht?\n");
        });

    // output pid
    printf("Pid: %d\n", process->pid);

    // create message
    message_message* message = NULL;
    message_message* message1 = malloc(sizeof(message_message));
    message_message* message2 = malloc(sizeof(message_message));

    // enqueue message
    message_queue_put(process->queue, message1);
    message_queue_put(process->queue, message2);

    // debug strings
    do {
        printf("First: %p\n", process->queue->first);
        printf("Last: %p\n", process->queue->last);

        message = message_queue_get(process->queue, 1.0f);
        printf("Current: %p\n", message);

        free(message);
    } while(message != NULL);

    // sleep a bit
    sleep(1);

    // cleanup
    process_cleanup(process);

    return 0;
}
