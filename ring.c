#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <dispatch/dispatch.h>
#include <mach/mach_time.h>
#include "node.h"
#include "message.h"
#include "process.h"

#define RING_COMMAND_SET_LISTENER (1)
#define RING_COMMAND_BENCHMARK (3)
#define RING_COMMAND_FINISHED (4)

typedef unsigned int ring_command;

typedef struct {
    actor_process_id_t sender;
    ring_command command;
    actor_process_id_t listener;
} ring_message;

void main_process(actor_process_t main) {
    // circles
    int circles = 10;
    int processes = 10;

    // parent pid
    actor_process_id_t parent = main->pid;

    // process function
    actor_process_function_t function = ^(actor_process_t self) {
            ring_message answer;

            // memory for round
            int round = circles;

            // listener
            actor_process_id_t listener = main->pid;

            // main loop
            while (round >= 0) {
                // receive message
                actor_message_t message = actor_message_receive(self, 10.0);

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
                    actor_message_send(self, listener, actor_message_create(&answer,
                        sizeof(ring_message)));

                    round -= 1;
                }

                // release message
                actor_message_release(message);
            }

            // send message to main process
            answer.command = RING_COMMAND_FINISHED;
            answer.sender = self->pid;
            actor_message_send(self, parent,
                actor_message_create(&answer, sizeof(ring_message)));

        };

    // start processes
    actor_process_id_t* pids = malloc(processes * sizeof(actor_process_id_t));

    for (int i = 0; i < processes; i++) {
        pids[i] = actor_process_spawn(main->node, function);
    }

    // set listener
    ring_message c;
    c.sender = main->pid;
    c.command = RING_COMMAND_SET_LISTENER;

    for (int i = 0; i < processes - 1; i++) {
        c.listener = pids[i + 1];
        actor_message_send(main, pids[i], actor_message_create(&c, sizeof(ring_message)));
    }

    // close ring
    c.listener = pids[0];
    actor_message_send(main, pids[processes - 1], actor_message_create(&c, sizeof(ring_message)));

    // start benchmark
    c.command = RING_COMMAND_BENCHMARK;
    actor_message_send(main, pids[0], actor_message_create(&c, sizeof(ring_message)));

    // wait for and of benchmark
    actor_message_t response = actor_message_receive(main, 10.0);

    if (response == NULL) {
        return;
    }

    printf("Fertig\n");

    actor_message_release(response);
    free(pids);
}

double mach_elapsed_time(double start, double endTime)
{
    uint64_t diff = endTime - start;
    static double conversion = 0.0;

    if (conversion == 0.0) {
        mach_timebase_info_data_t info;
        kern_return_t err = mach_timebase_info(&info);

        if (err == 0)
            conversion = 1e-9 * (double) info.numer / (double) info.denom;
    }

    return conversion * (double) diff;
}

int main(int argc, char* argv[]) {
    // create node
    actor_node_t node = actor_node_create(0, 1000);

    // time variables
    uint64_t start, end;

    // get start time
    start = mach_absolute_time();

    // start main process
    actor_main_process(node, ^(actor_process_t self) {
            main_process(self);
        });

    // get end time
    end = mach_absolute_time();

    // print execution time
    printf("Execution Time: %f milliseconds\n", mach_elapsed_time(start, end) * 1000.0);

    // cleanup
    sleep(1);
    actor_node_release(node);

    return 0;
}
