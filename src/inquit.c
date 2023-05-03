#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <sys/socket.h>

#include "setup.h"
#include "command.h"


extern User *users;


int main() {
    int socket_fd = setup();
    printf("server: Successfully initialized socket on file descriptor %d.\n", socket_fd);

    // Initialize a signal handler for SIGINT
    struct sigaction sigint_action;
    sigint_action.sa_handler = &sigint_handler;
    sigint_action.sa_flags = 0;
    sigemptyset(&(sigint_action.sa_mask));
    if (sigaction(SIGINT, &sigint_action, NULL) == -1) {
        system_error(socket_fd, "server: sigaction");
    }

    // Initialize a set of all file descriptors
    int max_fd = socket_fd;
    fd_set all_fds;
    FD_ZERO(&all_fds);
    FD_SET(socket_fd, &all_fds);

    while (1) {
        // Get ready file descriptors
        fd_set ready_fds = all_fds;
        printf("server: Blocking on select...\n");
        if (select(max_fd + 1, &ready_fds, NULL, NULL, NULL) == -1) {
            system_error(socket_fd, "server: select");
        }

        // Create a new connection if the socket file descriptor is ready
        if (FD_ISSET(socket_fd, &ready_fds)) {
            printf("server: Accepting a new connection.\n");
            int client_fd = accept(socket_fd, NULL, NULL);
            if (client_fd == -1) {
                system_error(socket_fd, "server: accept");
            }
            init_user(socket_fd, client_fd);
            if (client_fd > max_fd) {
                max_fd = client_fd;
            }
            FD_SET(client_fd, &all_fds);

            // Greet the new user and prompt them for a username
            prompt(socket_fd, client_fd, "Welcome to Inquit! Please enter a username.\n");
        }

        // Check if any client file descriptors are ready
        User *curr = users;
        while (curr != NULL) {
            if (FD_ISSET(curr->fd, &ready_fds) && curr->fd != -1) {
                int open = process_input(socket_fd, curr);
                if (open == -1) {
                    printf("server: Client %d closed.\n", curr->fd);
                    close(curr->fd);
                    FD_CLR(curr->fd, &all_fds);
                    curr->fd = -1;
                }
            }
            curr = curr->next;
        }

        // Remove all disconnected users
        curr = users;
        User *prev = users;
        while (curr != NULL) {
            if (curr->fd != -1) {
                prev = curr;
                curr = curr->next;
                continue;
            }
            if (curr == prev) {
                users = curr->next;
                free_user(curr);
                curr = users;
                prev = users;
            } else {
                prev->next = curr->next;
                free_user(curr);
                curr = prev->next;
            }
        }
    }

    fprintf(stderr, "server: Something weird happened.\n");
    close(socket_fd);
    exit(1);
}
