#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include "netdb.h"
#include <string.h>
#include "actor.h"

// message send process
void actor_distributer_message_send(actor_process_t self, int sock) {
    // create header
    actor_distributer_header_struct header;

    // send loop
    while (true) {
        // get message
        actor_message_t actor_message = actor_message_receive(self, 10.0);

        // check timeout
        if (actor_message == NULL) {
            // quit connection
            header.quit = true;
            send(sock, &header, sizeof(actor_distributer_header_struct), 0);
            break;
        }

        // get distributer message
        actor_distributer_message_t distributer_message =
            (actor_distributer_message_t)(actor_message->data);
        printf("%p\n", distributer_message->message);

        // get message to send
        actor_message_data_t data = distributer_message->message->data;
        printf("%p\n", data);

        // create header
        header.dest_id = distributer_message->dest_id;
        header.message_size = distributer_message->message->size;
        header.quit = false;

        // send header
        send(sock, &header, sizeof(actor_distributer_header_struct), 0);

        // send message
        send(sock, data, distributer_message->message->size, 0);

        // release message
        // free(distributer_message);
        // free(data);
        // actor_message_release(actor_message);
    }

    // close connection
    close(sock);
}

// message receive process
void actor_distributer_message_receive(actor_process_t self, int sock) {
    int bytes_received;
    actor_distributer_header_struct header;

    // get messages
    while (true) {
        // receive header
        bytes_received = recv(sock, &header,
            sizeof(actor_distributer_header_struct), 0);

        // check for closed connection
        if (bytes_received <= 0) {
            printf("Connection closed!\n");
            close(sock);
            return;
        }

        // check correct header
        if (bytes_received != sizeof(actor_distributer_header_struct)) {
            printf("Wrong header size!\n");
            continue;
        }

        // check quit flag
        if (header.quit == true) {
            printf("Quit!\n");
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
            printf("Wrong message size!\n");
            continue;
        }

        // send message
        actor_message_send(self, self->node->nid, header.dest_id,
            data, header.message_size);

        // free message buffer
        free(data);
    }
}

// connect to node
actor_node_id_t actor_distributer_connect_to_node(actor_node_t node,
    char* const host_name, unsigned int port) {
    // check valid node
    if (node == NULL) {
        return -1;
    }

    // create client socket
    int bytes_received;
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    // check success
    if (sock == -1) {
        return -1;
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
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        return -1;
    }

    // send node id
    send(sock, &node->nid, sizeof(actor_node_id_t), 0);

    // get node id
    actor_node_id_t node_id;
    bytes_received = recv(sock, &node_id, sizeof(actor_node_id_t), 0);

    // check success
    if ((bytes_received != sizeof(actor_node_id_t)) || (node->remote_nodes[node_id] != -1)){
        // close connection
        close(sock);
        return -1;
    }

    // create send process
    actor_process_id_t sender = actor_process_spawn(node, ^(actor_process_t self) {
            actor_distributer_message_send(self, sock);
        });

    // create receive process
    actor_process_id_t receiver = actor_process_spawn(node, ^(actor_process_t self) {
            actor_distributer_message_receive(self, sock);
        });

    // save connector
    node->remote_nodes[node_id] = sender;

    return node_id;
}

// listen incomming connections
actor_node_id_t actor_distributer_listen(actor_node_t node, unsigned int port) {
    // check valid node
    if (node == NULL) {
        return -1;
    }

    // create server socket
    int bytes_received;
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    // check success
    if (sock == -1) {
        return -1;
    }

    // create server address struct
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY; 
    bzero(&(server_addr.sin_zero),8);

    // bind socket to address
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        return -1;
    }

    // start listening
    if (listen(sock, 5)) {
        return -1;
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
    if ((bytes_received != sizeof(actor_node_id_t)) || (node->remote_nodes[node_id] != -1)){
        // close connection
        close(connected);
        return -1;
    }

    // create send process
    actor_process_id_t sender = actor_process_spawn(node, ^(actor_process_t self) {
            actor_distributer_message_send(self, connected);
        });

    // create receive process
    actor_process_id_t receiver = actor_process_spawn(node, ^(actor_process_t self) {
            actor_distributer_message_receive(self, connected);
        });

    // save connector
    node->remote_nodes[node_id] = sender;

    return node_id;
}
