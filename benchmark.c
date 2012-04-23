#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "actor/actor.h"

void main_process(actor_process_t main) {
    // process id memory
    int processes = 100;
    actor_process_id_t* pids = malloc(processes * sizeof(actor_process_id_t));

    // process function
    actor_process_function_t function = ^(actor_process_t self) {
            // answer buffer
            char buffer[100];

            // create answer
            sprintf(buffer, "Hallo main, ich bin Prozess %d!", self->pid);

            // send message to main
            actor_message_send(self, main->pid, buffer, strlen(buffer) + 1);
        };

    // create processes
    for (int i = 0; i < processes; i++) {
        actor_process_spawn(main->node, function);
    }

    // receive messages
    actor_message_t message = NULL;
    for (int i = 0; i < processes; i++) {
        // get message
        message = actor_message_receive(main, 1.0);

        // output message
        if (message != NULL) {
            printf("%s\n", (char*)message->data);
        }
        else {
            printf("Message not received!\n");
        }

        // release message
        actor_message_release(message);
    }

    // wait
    actor_process_sleep(main, 2.0);
    printf("After sleep!\n");
}

int main(int argc, char* argv[]) {
    // create node
    actor_node_t node = actor_node_create(0, 10000);

    // time variables
    clock_t start, end;

    // get start time
    start = clock();

    // start main process
    actor_process_spawn(node, ^(actor_process_t self) {
            main_process(self);
        });

    // cleanup
    actor_node_release(node);

    // get end time
    end = clock();

    // print execution time
    printf("Execution Time: %f milliseconds\n", (double)(end - start) * 1000.0 / CLOCKS_PER_SEC);

    return 0;
}
