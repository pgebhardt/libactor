#ifndef DISTRIBUTER_H
#define DISTRIBUTER_H

// message header
typedef struct {
    actor_process_id_t dest_id;
    actor_size_t message_size;
    actor_data_type_t type;
} actor_distributer_header_s;
typedef actor_distributer_header_s* actor_distributer_header_t;

// connect to node
actor_error_t actor_distributer_connect_to_node(actor_node_t node, actor_node_id_t* nid,
    char* const host_name, unsigned int port, const char* key);

// listen incomming connections
actor_error_t actor_distributer_listen(actor_node_t node, actor_node_id_t* nid,
    unsigned int port, const char* key);

#endif