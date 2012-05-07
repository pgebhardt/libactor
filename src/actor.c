#include "actor.h"

// interface functions

// spawn process
actor_error_t actor_spawn(actor_node_t node, actor_process_id_t* pid,
    actor_process_function_t function) {
    // call method
    return actor_node_spawn_process(node, pid, function);
}

// message sendig
actor_error_t actor_send(actor_process_t process,
    actor_node_id_t destination_nid, actor_process_id_t destination_pid,
    actor_data_type_t type, actor_message_data_t const data, actor_size_t size) {
    // call method
    return actor_node_send_message(process->node, destination_nid, destination_pid,
    type, data, size);
}

// message receive
actor_error_t actor_receive(actor_process_t process, actor_message_t* message,
    actor_time_t timeout) {
    // call method
    return actor_process_receive_message(process, message, timeout);
}
