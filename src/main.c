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
    message = process->queue->first;
    int i;

    while (message != NULL) {
        i = 0;
        printf("First: %p\n", process->queue->first);
        printf("Last: %p\n", process->queue->last);

        while (message != NULL) {
            printf("%d: %p\n", i, message);
            message = (message_message*)message->next;
            i++;
        }
        message = message_queue_get(process->queue);
    }

    // sleep a bit
    sleep(1);

    // cleanup
    process_cleanup(process);

    return 0;
}
