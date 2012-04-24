#ifndef DISTRIBUTER_H
#define DISTRIBUTER_H

// message header
typedef struct {
    actor_process_id_t dest_id;
    actor_size_t message_size;
    bool quit;
} actor_distributer_header_struct;
typedef actor_distributer_header_struct* actor_distributer_header_t;

// connect to node
actor_error_t actor_distributer_connect_to_node(actor_node_t node, actor_node_id_t* nid,
    char* const host_name, unsigned int port);

// listen incomming connections
actor_error_t actor_distributer_listen(actor_node_t node, actor_node_id_t* nid,
    unsigned int port);

#endif
