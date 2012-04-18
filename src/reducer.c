#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "message.h"
#include "node.h"
#include "process.h"

// reducer
void reduce(process_process* self, process_id parent, double* list, int size) {
    double result = 0.0;

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
        process_id pid1 = node_process_spawn(self->node,
            ^(process_process* s) {
                reduce(s, self->pid, list, split * sizeof(double));
        });

        process_id pid2 = node_process_spawn(self->node,
            ^(process_process* s) {
                reduce(s, self->pid, &(list[split]),
                    size - (split * sizeof(double)));
        });

        // gather results
        message_message* result1 = process_message_receive(self, 5.0f);
        message_message* result2 = process_message_receive(self, 5.0f);

        // check for success
        if ((result1 == NULL) || (result2 == NULL)) {
            printf("Process: %d, Did not get result from reducer!\n", self->pid);
            return;
        }

        // reduce result
        result = *(double*)result1->message_data +
            *(double*)result2->message_data;

        // cleanup messages
        message_message_release(result1);
        message_message_release(result2);
    }

    // send result to parent
    process_message_send(self, parent,
        message_message_create(&result, sizeof(double)));
}

void main_process(process_process* self) {
    // data
    int size = 1000;
    double* list = malloc(size * sizeof(double));

    for (int i = 0; i < size; i++) {
        list[i] = (double)i;
    }

    // start first reducer
    process_id reducer = node_process_spawn(self->node, ^(process_process* s) {
            reduce(s, self->pid, list, size * sizeof(double));
        });

    // get result
    message_message* result = process_message_receive(self, 5.0f);

    // print result
    if (result != NULL) {
        printf("Result: %f\n", *(double*)result->message_data);
    }
    else {
        printf("Invalid result!\n");
    }

    free(list);
}

int main(int argc, char* argv[]) {
    // create node
    node_node* node = node_create(0, 100);

    // spawn main process
    node_main_process(node, ^(process_process* self) {
            main_process(self);
        });

    // cleanup
    node_release(node);

    return 0;
}
