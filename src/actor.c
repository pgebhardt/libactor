// libactor
//
// Implementation of an erlang style actor model using libdispatch
// Copyright (C) 2012  Patrik Gebhardt
// Contact: patrik.gebhardt@rub.de
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "actor.h"

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
