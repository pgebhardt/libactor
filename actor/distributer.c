#include <sys/types.h>
#include <sys/socket.h>
//#include <netinet/in.h>
#include "netdb.h"
#include <string.h>
#include "actor.h"
#include "distributer.h"

// connect to node
actor_process_id_t actor_distributer_connect_to_node(actor_node_t node,
    char* const host_name, unsigned int port) {
    // check valid node
    if (node == NULL) {
        return -1;
    }

    // create client socket
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

    // create connection process
    actor_process_id_t connector = actor_process_spawn(node, ^(actor_process_t self) {
            int bytes_received;
            actor_distributer_header_struct header;


            // send loop
            while (true) {
                // get message
                actor_message_t actor_message = actor_message_receive(self, 10.0);

                // check timeout
                if (actor_message == NULL) {
                    // quit connection
                    header.quit = true;
                    send(sock, &header, sizeof(header), 0);
                    break;
                }

                // get distributer message
                actor_distributer_message_t distributer_message =
                    (actor_distributer_message_t)actor_message->data;

                // create header
                header.dest_id = distributer_message->dest_id;
                header.message_size = distributer_message->message->size;
                header.quit = false;

                // send header
                send(sock, &header, sizeof(header), 0);

                // send message
                send(sock, distributer_message->message->data, header.message_size, 0);

                // release message
                actor_message_release(distributer_message->message);
                actor_message_release(actor_message);
            }

            // close connection
            close(sock);
        });

    return connector;
}

// listen incomming connections
actor_process_id_t actor_distributer_listen(actor_node_t node, unsigned int port) {
    // check valid node
    if (node == NULL) {
        return -1;
    }

    // create server socket
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

    // create listening process
    actor_process_id_t listener = actor_process_spawn(node, ^(actor_process_t self) {
            int bytes_received;
            actor_distributer_header_struct header;

            // start listening
            if (listen(sock, 5)) {
                return;
            }

            // accept incomming connections
            unsigned int sin_size = sizeof(struct sockaddr_in);
            struct sockaddr_in client_addr;
            int connected = accept(sock, (struct sockaddr *)&client_addr,&sin_size);

            // get messages
            while (true) {
                // receive header
                bytes_received = recv(connected, &header,
                    sizeof(actor_distributer_header_struct), 0);

                // check correct header
                if (bytes_received != sizeof(actor_distributer_header_struct)) {
                    continue;
                }

                // check quit flag
                if (header.quit == true) {
                    // close connection
                    close(connected);
                    break;
                }

                // create message buffer
                char data[header.message_size];

                // receive message
                bytes_received = recv(connected, data, header.message_size, 0);

                // check correct message length
                if (bytes_received != header.message_size) {
                    continue;
                }

                // send message
                actor_message_send(self, header.dest_id, data, header.message_size);
            }

            // close socket
            close(sock);
        });

    return listener;
}
