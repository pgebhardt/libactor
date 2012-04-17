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
                reduce(s, self->pid, &(list[split]),
                    size - (split * sizeof(double)));
        });

        // check for success
        if ((process1 == NULL) || (process2 == NULL)) {
            printf("Process: %d, Could not spawn reducer!\n", self->pid);
            return;
        }

        // gather results
        message_message* result1 = process_message_receive(self, 5.0f);
        message_message* result2 = process_message_receive(self, 5.0f);

        // check for success
        if ((result1 == NULL) || (result2 == NULL)) {
            printf("Process: %d, Did not get result from reducer!\n", self->pid);
            return;
        }

        // debug string
        printf("Process: %d, Data: %f, %f\n", self->pid,
            *(double*)result1->message_data, *(double*)result2->message_data);

        // reduce result
        result = *(double*)result1->message_data +
            *(double*)result2->message_data;

        // cleanup messages
        message_message_cleanup(result1);
        message_message_cleanup(result2);
    }

    // send result to parent
    process_message_send(self, parent,
        message_message_create(&result, sizeof(double)));
}

void main_process(process_process* self) {
    // data
    double* list = malloc(5 * sizeof(double));
    list[0] = 1.0;
    list[1] = 2.0;
    list[2] = 3.0;
    list[3] = 4.0;
    list[4] = 5.0;

    // start first reducer
    process_process* reducer = process_spawn(self->process_node, ^(process_process* s) {
            reduce(s, self->pid, list, 5 * sizeof(double));
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
    node_node* node = node_create(0, 3);

    // spawn main process
    process_spawn(node, ^(process_process* self) {
            main_process(self);
        });

    sleep(10);

    node_cleanup(node);

    return 0;
}
