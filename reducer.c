#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "actor/actor.h"

// reducer
void reduce(actor_process_t self, actor_process_id_t parent, double* list, int size) {
    double result = 0.0;
    printf("Process %d started!\n", self->pid);

    // check length of list
    if (size == sizeof(double)) {
        result = *list;
    }
    else if (size == 2 * sizeof(double)) {
        // calc result
        result = list[0] + list[1];
    }
    else {
        // split data
        int split = (size / sizeof(double)) / 2;

        // spawn reducer
        actor_process_id_t pid1 = actor_process_spawn(self->node,
            ^(actor_process_t s) {
                reduce(s, self->pid, list, split * sizeof(double));
        });

        actor_process_id_t pid2 = actor_process_spawn(self->node,
            ^(actor_process_t s) {
                reduce(s, self->pid, &(list[split]),
                    size - (split * sizeof(double)));
        });

        // gather results
        actor_message_t result1 = actor_message_receive(self, 5.0);
        actor_message_t result2 = actor_message_receive(self, 5.0);

        // check for success
        if ((result1 == NULL) || (result2 == NULL)) {
            printf("Process: %d, Did not get result from reducer!\n", self->pid);
            return;
        }

        // reduce result
        result = *(double*)result1->data +
            *(double*)result2->data;

        // cleanup messages
        actor_message_release(result1);
        actor_message_release(result2);
    }

    // send result to parent
    actor_message_send(self, parent, &result, sizeof(double));
}

void main_process(actor_process_t self) {
    // data
    int size = 50;
    double* list = malloc(size * sizeof(double));

    for (int i = 0; i < size; i++) {
        list[i] = (double)i;
    }

    // start first reducer
    actor_process_id_t reducer = actor_process_spawn(self->node, ^(actor_process_t s) {
            reduce(s, self->pid, list, size * sizeof(double));
        });

    // get result
    actor_message_t result = actor_message_receive(self, 5.0);

    // print result
    if (result != NULL) {
        printf("Result: %f\n", *(double*)result->data);
    }
    else {
        printf("Invalid result!\n");
    }

    free(list);
}

int main(int argc, char* argv[]) {
    // create node
    actor_node_t node = actor_node_create(0, 10000);

    // spawn main process
    actor_process_spawn(node, ^(actor_process_t self) {
            main_process(self);
        });

    // cleanup
    actor_node_release(node);

    return 0;
}
