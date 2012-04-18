#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "node.h"
#include "message.h"
#include "process.h"

#define RING_COMMAND_SET_LISTENER (1)
#define RING_COMMAND_BENCHMARK (3)
#define RING_COMMAND_FINISHED (4)

typedef unsigned int ring_command;

typedef struct {
    process_id sender;
    ring_command command;
    process_id listener;
} ring_message;

void main_process(process_process* main) {
    // circles
    int circles = 2;
    int processes = 8;

    // parent pid
    process_id parent = main->pid;

    // process function
    process_process_function function = ^(process_process* self) {
            ring_message answer;

            // memory for round
            int round = circles;

            // listener
            process_id listener = main->pid;

            // main loop
            while (round >= 0) {
                // receive message
                message_message* message = process_message_receive(self, 10.0);

                if (message == NULL) {
                    break;
                }

                ring_message* command = (ring_message*)message->message_data;

                // execute command
                if (command->command == RING_COMMAND_SET_LISTENER) {
                    listener = command->listener;
                }
                else if (command->command == RING_COMMAND_BENCHMARK) {
                    printf("Process: %d, bechmark, round %d\n", self->pid, round);
                    answer.sender = self->pid;
                    answer.command = RING_COMMAND_BENCHMARK;
                    process_message_send(self, listener, message_message_create(&answer,
                        sizeof(ring_message)));

                    round -= 1;
                }

                // release message
                message_message_release(message);
            }

            // send message to main process
            answer.command = RING_COMMAND_FINISHED;
            answer.sender = self->pid;
            process_message_send(self, parent,
                message_message_create(&answer, sizeof(ring_message)));

        };

    // start processes
    process_id *pids = malloc(processes * sizeof(process_id));

    for (int i = 0; i < processes; i++) {
        pids[i] = node_process_spawn(main->node, function);
    }

    // set listener
    ring_message c;
    c.sender = main->pid;
    c.command = RING_COMMAND_SET_LISTENER;

    for (int i = 0; i < processes - 1; i++) {
        c.listener = pids[i + 1];
        process_message_send(main, pids[i], message_message_create(&c, sizeof(ring_message)));
    }

    // close ring
    c.listener = pids[0];
    process_message_send(main, pids[processes - 1], message_message_create(&c, sizeof(ring_message)));

    // start benchmark
    c.command = RING_COMMAND_BENCHMARK;
    process_message_send(main, pids[0], message_message_create(&c, sizeof(ring_message)));

    // wait for and of benchmark
    message_message* response = process_message_receive(main, 10.0);

    if (response == NULL) {
        return;
    }

    printf("Fertig\n");

    message_message_release(response);
    free(pids);
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
    sleep(1);
    node_release(node);

    return 0;
}
