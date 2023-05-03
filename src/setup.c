#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "setup.h"
#include "command.h"


/*
Terminate the server due to a system call error.
*/
void system_error(int socket_fd, char *message) {
    perror(message);
    close(socket_fd);
    exit(1);
}


/*
Set up a socket to lister for connections.
Return the file descriptor of the socket or exit if an error occurs.
*/
int setup() {
    // Create the socket file descriptor
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("server: socket");
        exit(1);
    }

    // Ensure the port can be reused immediately after termination of the server
    int on = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (const char *) &on, sizeof(on)) == -1) {
        system_error(socket_fd, "server: setsockopt");
    }

    // Create the address of the socket
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = INADDR_ANY;
    memset(&(address.sin_zero), 0, 8);

    // Assign the address to the socket
    if (bind(socket_fd, (struct sockaddr *) &address, sizeof(struct sockaddr_in)) == -1) {
        system_error(socket_fd, "server: bind");
    }

    // Listen for connections to the socket
    if (listen(socket_fd, MAX_BACKLOG) == -1) {
        system_error(socket_fd, "server: listen");
    }

    return socket_fd;
}


/*
Terminate the program when a SIGINT is received.
*/
void sigint_handler(int code) {
    printf("server: SIGINT caught. Freeing memory...\n");
    free_users();
    printf("server: Terminating...\n");
    exit(0);
}
