#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include "actor.h"

// message send process
actor_error_t actor_distributer_message_send(actor_process_t self, int sock) {
    // create header
    actor_distributer_header_struct header;

    // message pointer
    actor_message_t message = NULL;

    // send loop
    while (true) {
        // get message
        actor_message_t message = NULL;
        if (actor_message_receive(self, &message, 10.0) != ACTOR_SUCCESS) {
            // quit connection
            header.quit = true;
            send(sock, &header, sizeof(actor_distributer_header_struct), 0);

            // close socket
            close(sock);

            return ACTOR_ERROR_TIMEOUT;
        }

        // create header
        header.dest_id = message->destination;
        header.message_size = message->size;
        header.quit = false;

        // send header
        send(sock, &header, sizeof(actor_distributer_header_struct), 0);

        // send message
        send(sock, message->data, message->size, 0);

        // release message
        actor_message_release(message);
    }

    // close connection
    close(sock);

    return ACTOR_SUCCESS;
}

// message receive process
actor_error_t actor_distributer_message_receive(actor_process_t self, int sock) {
    int bytes_received;
    actor_distributer_header_struct header;

    // get messages
    while (true) {
        // receive header
        bytes_received = recv(sock, &header,
            sizeof(actor_distributer_header_struct), 0);

        // check for closed connection
        if (bytes_received <= 0) {
            close(sock);
            return ACTOR_ERROR_NETWORK;
        }

        // check correct header
        if (bytes_received != sizeof(actor_distributer_header_struct)) {
            continue;
        }

        // check quit flag
        if (header.quit == true) {
            // close connection
            close(sock);
            break;
        }

        // create message buffer
        char* data = malloc(header.message_size);

        // receive message
        bytes_received = recv(sock, data, header.message_size, 0);

        // check correct message length
        if (bytes_received != header.message_size) {
            continue;
        }

        // send message
        actor_message_send(self, self->node->nid, header.dest_id,
            data, header.message_size);

        // free message buffer
        free(data);
    }

    return ACTOR_SUCCESS;
}

// connect to node
actor_error_t actor_distributer_connect_to_node(actor_node_t node, actor_node_id_t* nid,
    char* const host_name, unsigned int port) {
    // check valid node
    if (node == NULL) {
        return ACTOR_ERROR_INVALUE;
    }

    // init node id pointer
    if (nid != NULL) {
        *nid = ACTOR_INVALID_ID;
    }

    // create client socket
    int bytes_received;
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    // check success
    if (sock == -1) {
        return ACTOR_ERROR_NETWORK;
    }

    // get host address
    struct hostent* host = gethostbyname(host_name);

    // create server address struct
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    bzero(&(server_addr.sin_zero),8);

    // connect
    if (connect(sock, (struct sockaddr *)&server_addr,
            sizeof(struct sockaddr)) == -1) {
        return ACTOR_ERROR_NETWORK;
    }

    // send node id
    send(sock, &node->nid, sizeof(actor_node_id_t), 0);

    // get node id
    actor_node_id_t node_id;
    bytes_received = recv(sock, &node_id, sizeof(actor_node_id_t), 0);

    // check success
    if ((bytes_received != sizeof(actor_node_id_t)) ||
        (node->remote_nodes[node_id] != ACTOR_INVALID_ID)) {
        // close connection
        close(sock);
        return ACTOR_ERROR_NETWORK;
    }

    // create send process
    actor_process_id_t sender = ACTOR_INVALID_ID;
    actor_process_spawn(node, &sender,
        ^actor_error_t(actor_process_t self) {
            return actor_distributer_message_send(self, sock);
        });

    // create receive process
    actor_process_spawn(node, NULL, ^actor_error_t(actor_process_t self) {
            return actor_distributer_message_receive(self, sock);
        });

    // save connector
    node->remote_nodes[node_id] = sender;

    // set node id
    if (nid != NULL) {
        *nid = node_id;
    }

    return ACTOR_SUCCESS;
}

// listen incomming connections
actor_error_t actor_distributer_listen(actor_node_t node, actor_node_id_t* nid,
    unsigned int port) {
    // check valid node
    if (node == NULL) {
        return ACTOR_ERROR_INVALUE;
    }

    // init node id pointer
    if (nid != NULL) {
        *nid = ACTOR_INVALID_ID;
    }

    // create server socket
    int bytes_received;
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    // check success
    if (sock == -1) {
        return ACTOR_ERROR_NETWORK;
    }

    // create server address struct
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY; 
    bzero(&(server_addr.sin_zero),8);

    // bind socket to address
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        return ACTOR_ERROR_NETWORK;
    }

    // start listening
    if (listen(sock, 5)) {
        return ACTOR_ERROR_NETWORK;
    }

    // accept incomming connections
    unsigned int sin_size = sizeof(struct sockaddr_in);
    struct sockaddr_in client_addr;
    int connected = accept(sock, (struct sockaddr *)&client_addr,&sin_size);

    // close socket
    close(sock);

    // send node id
    send(connected, &node->nid, sizeof(actor_node_id_t), 0);

    // get node id
    actor_node_id_t node_id;
    bytes_received = recv(connected, &node_id, sizeof(actor_node_id_t), 0);

    // check success
    if ((bytes_received != sizeof(actor_node_id_t)) ||
        (node->remote_nodes[node_id] != -1)){
        // close connection
        close(connected);
        return ACTOR_ERROR_NETWORK;
    }

    // create send process
    actor_process_id_t sender = ACTOR_INVALID_ID;
    actor_process_spawn(node, &sender, ^actor_error_t(actor_process_t self) {
            return actor_distributer_message_send(self, connected);
        });

    // create receive process
    actor_process_spawn(node, NULL, ^actor_error_t(actor_process_t self) {
            return actor_distributer_message_receive(self, connected);
        });

    // save connector
    node->remote_nodes[node_id] = sender;

    return node_id;
}
