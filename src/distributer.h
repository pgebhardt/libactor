#ifndef DISTRIBUTER_H
#define DISTRIBUTER_H

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
