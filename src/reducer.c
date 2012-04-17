#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "message.h"
#include "node.h"
#include "process.h"

// reducer
void reduce(process_process* self, process_id parent, double* list, int size) {
    double result = 0.0;
    printf("Process: %d, Parent: %d, Size: %d\n", self->pid, parent, size);

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
        printf("Process: %d, Split: %d, Size: %d\n", self->pid,
            split, (int)(size / sizeof(double)));

        // spawn reducer
        process_process* process1 = process_spawn(self->process_node,
            ^(process_process* s) {
                reduce(s, self->pid, list, split * sizeof(double));
        });

        process_process* process2 = process_spawn(self->process_node,
            ^(process_process* s) {
                reduce(s, self->pid, &(list[split - 1]), size - (split * sizeof(double)));
        });

        // gather results
        message_message* result1 = process_message_receive(self, 5.0f);
        message_message* result2 = process_message_receive(self, 5.0f);

        // debug string
        printf("Process: %d, Data: %f, %f\n", self->pid, *(double*)result1->message_data,
            *(double*)result2->message_data);
        // reduce result
        result = *(double*)result1->message_data +
            *(double*)result2->message_data;

        // cleanup messages
        message_message_cleanup(result1);
        message_message_cleanup(result2);

        // cleanup processes
        process_cleanup(process1);
        process_cleanup(process2);
    }

    // send result to parent
    if (parent != 10000) {
        process_message_send(self->process_node, parent, message_message_create(
            &result, sizeof(double)));
    }
    else {
        printf("result: %f\n", result);
    }
}


int main(int argc, char* argv[]) {
    // create node
    node_node* node = node_create(0, 100);

    double* list = malloc(5 * sizeof(double));
    list[0] = 1.0;
    list[1] = 2.0;
    list[2] = 3.0;
    list[3] = 4.0;
    list[4] = 5.0;

    // start first reducer
    process_process* reducer = process_spawn(node, ^(process_process* self) {
            reduce(self, 10000, list, 5 * sizeof(double));
        });

    sleep(5);

    free(list);
    process_cleanup(reducer);
    node_cleanup(node);

    return 0;
}
