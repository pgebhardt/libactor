#include <stdio.h>
#include "actor/actor.h"

actor_error_t main_process(actor_process_t main) {
    // start dummy process
    actor_process_id_t pid;
    actor_error_t err = actor_process_spawn(main->node, &pid,
        ^actor_error_t(actor_process_t self) {
            // link to main
            actor_process_link(self, main->node->nid, main->pid);

            // die with error
            return ACTOR_FAILURE;
        });

    // receive message
    actor_message_t message = actor_message_receive(main, 10.0);

    // check success
    if (message == NULL) {
        return ACTOR_FAILURE;
    }

    // cast message to error message
    actor_process_error_message_t error_message = (actor_process_error_message_t)message->data;

    // print error
    printf("%d.%d died with %d!\n", error_message->nid, error_message->pid, error_message->error);

    // release message
    actor_message_release(message);

    return ACTOR_SUCCESS;
}

int main(int argc, char* argv[]) {
    // create node
    actor_node_t node = actor_node_create(0, 1000);

    // spawn main process
    actor_process_spawn(node, NULL, ^actor_error_t(actor_process_t self) {
            return main_process(self);
        });

    // release node
    actor_node_release(node);

    return 0;
}
