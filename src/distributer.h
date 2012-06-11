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

#ifndef ACTOR_DISTRIBUTER_H
#define ACTOR_DISTRIBUTER_H

// key length
#define ACTOR_DISTRIBUTER_KEYLENGTH (30)

// message header
typedef struct {
    actor_process_id_t dest_id;
    actor_size_t message_size;
    actor_data_type_t type;
} actor_distributer_header_s;
typedef actor_distributer_header_s* actor_distributer_header_t;

// connect to node
actor_error_t actor_distributer_connect_to_node(actor_node_t node, actor_node_id_t* nid,
    const char* host_name, unsigned int port, const char* key);

// listen incomming connections
actor_error_t actor_distributer_listen(actor_node_t node, actor_node_id_t* nid,
    unsigned int port, const char* key);

// disconnect from node
actor_error_t actor_distributer_disconnect_from_node(actor_node_t node, actor_node_id_t nid);

#endif
