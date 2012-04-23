#ifndef DISTRIBUTER_H
#define DISTRIBUTER_H

// message header
typedef struct {
    actor_process_id_t dest_id;
    actor_size_t message_size;
    bool quit;
} actor_distributer_header_struct;
typedef actor_distributer_header_struct* actor_distributer_header_t;

// distributer message
typedef struct {
    actor_process_id_t dest_id;
    actor_message_t message;
} actor_distributer_message_struct;
typedef actor_distributer_message_struct* actor_distributer_message_t;

// connect to node
actor_node_id_t actor_distributer_connect_to_node(actor_node_t node,
    char* const host_name, unsigned int port);

// listen incomming connections
actor_node_id_t actor_distributer_listen(actor_node_t node, unsigned int port);

#endif
