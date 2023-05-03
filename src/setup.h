#ifndef SETUP_H
#define SETUP_H


#ifndef PORT
    #define PORT 54879
#endif
#define MAX_BACKLOG 256


/*
Terminate the server due to a system call error.
*/
void system_error(int socket_fd, char *message);


/*
Set up a socket to lister for connections.
Return the file descriptor of the socket or exit if an error occurs.
*/
int setup();


/*
Terminate the program when a SIGINT is received.
*/
void sigint_handler(int code);


#endif // SETUP_H
