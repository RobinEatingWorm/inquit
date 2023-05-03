#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "setup.h"
#include "command.h"


User *users = NULL;


/*
Add a client with the given file descriptor to the list of users.
*/
void init_user(int socket_fd, int client_fd) {
    // Initialize a new user
    User *new_user = malloc(sizeof(User));
    if (new_user == NULL) {
        system_error(socket_fd, "server: malloc");
    }
    new_user->fd = client_fd;
    new_user->username = NULL;
    new_user->room = NULL;
    new_user->buf = malloc(sizeof(char) * BUF_SIZE);
    if (new_user->buf == NULL) {
        system_error(socket_fd, "server: malloc");
    }
    new_user->inbuf = 0;
    new_user->next = NULL;

    // Add the user to the end of the list
    if (users == NULL) {
        users = new_user;
    } else {
        User *curr = users;
        while (curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = new_user;
    }

    printf("server: Initialized new client on file descriptor %d.\n", client_fd);
}


/*
Free the memory of a user.
*/
void free_user(User *user) {
    free(user->username);
    free(user->room);
    free(user->buf);
    free(user);
}


/*
Free the memory of all users.
*/
void free_users() {
    User *curr = users;
    User *next = users;
    while (next != NULL) {
        curr = next;
        next = next->next;
        free_user(curr);
    }
}


/*
Search the characters in buf for '\r' followed by '\n'. Return one plus the index of the '\n',
or -1 if no newline is found.
*/
int get_newline_double(char *buf, int inbuf) {
    for (int i = 0; i + 1 < inbuf; i++) {
        if (buf[i] == '\r' && buf[i + 1] == '\n') {
            return i + 2;
        }
    }
    return -1;
}


/*
Search the characters in buf for either '\r' or '\n'. Return one plus the index of whichever
character is found, or -1 if no newline is found.
*/
int get_newline_single(char *buf, int inbuf) {
    for (int i = 0; i < inbuf; i++) {
        if (buf[i] == '\r' || buf[i] == '\n') {
            return i + 1;
        }
    }
    return -1;
}


/*
Add a username to a new user and prompt them to join a room.
*/
void add_username(int socket_fd, User *user, char *username) {
    // Save the user's username
    user->username = malloc(sizeof(char) * (strlen(username) + 1));
    if (user->username == NULL) {
        system_error(socket_fd, "server: malloc");
    }
    strcpy(user->username, username);
    user->username[strlen(username)] = '\0';
    printf("server: User %d added username <%s>.\n", user->fd, user->username);

    // Send a prompt back to the user
    char *message_1 = "Welcome to the lobby, ";
    char *message_2 = "!\n";
    int total_len = strlen(message_1) + strlen(user->username) + strlen(message_2);
    char message[total_len + 1];
    message[0] = '\0';
    strcat(message, message_1);
    strcat(message, user->username);
    strcat(message, message_2);
    message[total_len] = '\0';
    prompt(socket_fd, user->fd, message);
    prompt(socket_fd, user->fd, LOBBY_INST_0);
    prompt(socket_fd, user->fd, LOBBY_INST_1);
    prompt(socket_fd, user->fd, LOBBY_INST_2);
}


/*
Write a list of all users.
*/
void list_users(int socket_fd, User *user) {
    // Print a header
    prompt(socket_fd, user->fd, "Users:\n");

    // Print the user list
    User *curr = users;
    while (curr != NULL) {
        char *message_1 = " * ";
        char *message_2;
        char *message_3;
        int total_len;
        if (curr->room == NULL) {
            message_2 = " (lobby)\n";
            total_len = strlen(message_1) + strlen(curr->username) + strlen(message_2);
        } else {
            message_2 = " (room ";
            message_3 = ")\n";
            total_len = strlen(message_1) + strlen(curr->username) + strlen(message_2) + strlen(curr->room) + strlen(message_3);
        }
        char message[total_len + 1];
        message[0] = '\0';
        strcat(message, message_1);
        strcat(message, curr->username);
        strcat(message, message_2);
        if (curr->room != NULL) {
            strcat(message, curr->room);
            strcat(message, message_3);
        }
        message[total_len] = '\0';
        prompt(socket_fd, user->fd, message);
        curr = curr->next;
    }
}


/*
Write a list of all users in the same room as the user.
*/
void list_users_room(int socket_fd, User *user) {
    // Print a header
    char *message_1 = "Users in room ";
    char *message_2 = ":\n";
    int total_len = strlen(message_1) + strlen(user->room) + strlen(message_2);
    char message[total_len + 1];
    message[0] = '\0';
    strcat(message, message_1);
    strcat(message, user->room);
    strcat(message, message_2);
    message[total_len] = '\0';
    prompt(socket_fd, user->fd, message);

    // Print the user list
    User *curr = users;
    while (curr != NULL) {
        if (curr->room == NULL) {
            curr = curr->next;
            continue;
        }
        if (strcmp(user->room, curr->room) == 0) {
            char *message_1 = " * ";
            char *message_2 = "\n";
            int total_len = strlen(message_1) + strlen(curr->username) + strlen(message_2);
            char message[total_len + 1];
            message[0] = '\0';
            strcat(message, message_1);
            strcat(message, curr->username);
            strcat(message, message_2);
            message[total_len] = '\0';
            prompt(socket_fd, user->fd, message);
        }
        curr = curr->next;
    }
}


/*
Add a user to a room and prompt them to send messages.
*/
void join_room(int socket_fd, User *user, char *room) {
    // Save the user's room
    user->room = malloc(sizeof(char) * (strlen(room) + 1));
    if (user->room == NULL) {
        system_error(socket_fd, "server: malloc");
    }
    strcpy(user->room, room);
    user->room[strlen(room)] = '\0';
    printf("server: User %d joined room <%s>.\n", user->fd, user->room);

    // Send a prompt back to the user
    char *message_1 = "Joined room ";
    char *message_2 = ".\n";
    int total_len = strlen(message_1) + strlen(user->room) + strlen(message_2);
    char message[total_len + 1];
    message[0] = '\0';
    strcat(message, message_1);
    strcat(message, user->room);
    strcat(message, message_2);
    message[total_len] = '\0';
    prompt(socket_fd, user->fd, message);
    prompt(socket_fd, user->fd, ROOM_INST_0);
    prompt(socket_fd, user->fd, ROOM_INST_1);
}


/*
Remove a user from a room and prompt them to join a room.
*/
void leave_room(int socket_fd, User *user) {
    // Get the user's room
    char past_room[strlen(user->room) + 1];
    past_room[0] = '\0';
    strcat(past_room, user->room);
    past_room[strlen(user->room)] = '\0';
    printf("server: User %d left room <%s>.\n", user->fd, past_room);

    // Remove the user's room
    free(user->room);
    user->room = NULL;

    // Send a prompt back to the user
    char *message_1 = "Left room ";
    char *message_2 = ". You are currently in the lobby.\n";
    int total_len = strlen(message_1) + strlen(past_room) + strlen(message_2);
    char message[total_len + 1];
    message[0] = '\0';
    strcat(message, message_1);
    strcat(message, past_room);
    strcat(message, message_2);
    message[total_len] = '\0';
    prompt(socket_fd, user->fd, message);
    prompt(socket_fd, user->fd, LOBBY_INST_0);
    prompt(socket_fd, user->fd, LOBBY_INST_1);
    prompt(socket_fd, user->fd, LOBBY_INST_2);
}


/*
Send a message from the user to all other users in the same room.
*/
void post(int socket_fd, User *user, char *message) {
    // Create a string containing the user's username and the message
    int total_len = strlen(user->username) + strlen(message) + 3;
    char name_message[total_len + 1];
    name_message[0] = '\0';
    strcat(name_message, user->username);
    strcat(name_message, ": ");
    strcat(name_message, message);
    strcat(name_message, "\n");
    name_message[total_len] = '\0';
    printf("server: User %d posted <%s> to room <%s>.\n", user->fd, message, user->room);

    // Send the string to other users in the same room
    User *curr = users;
    while (curr != NULL) {
        if (curr->room == NULL || curr->fd == -1) {
            curr = curr->next;
            continue;
        }
        if (strcmp(user->room, curr->room) == 0) {
            prompt(socket_fd, curr->fd, name_message);
        }
        curr = curr->next;
    }
}


/*
Add user input to the user's buffer. If a network newline is detected, process the user's input.
Return 0 if successful or -1 if the file descriptor is closed.
*/
int process_input(int socket_fd, User *user) {
    // Add the bytes read to the user's buffer
    int bytes = read(user->fd, user->buf + user->inbuf, BUF_SIZE - user->inbuf);
    printf("server: %d of %d bytes in buffer of user %d.\n", user->inbuf, BUF_SIZE, user->fd);
    printf("server: Read %d bytes from user %d.\n", bytes, user->fd);
    if (bytes == 0) {
        return -1;
    } else if (bytes == -1) {
        system_error(socket_fd, "server: read");
    }
    user->inbuf += bytes;
    printf("server: %d of %d bytes in buffer of user %d.\n", user->inbuf, BUF_SIZE, user->fd);

    // Find the newline, if any
    int newline_position = get_newline_double(user->buf, user->inbuf);
    if (newline_position == -1) {
        newline_position = get_newline_single(user->buf, user->inbuf);
        if (newline_position == -1) {
            return 0;
        }
        user->buf[newline_position - 1] = '\0';
    } else {
        user->buf[newline_position - 2] = '\0';
    }

    // Extract a string containing the user input from the buffer
    
    char *input = malloc(sizeof(char) * (strlen(user->buf) + 1));
    if (input == NULL) {
        system_error(socket_fd, "server: malloc");
    }
    strcpy(input, user->buf);
    printf("server: User %d input: <%s>\n", user->fd, input);

    // Remove the input from the buffer
    memmove(user->buf, user->buf + newline_position, BUF_SIZE - newline_position);
    user->inbuf -= newline_position;
    printf("server: Removed %d bytes from buffer of user %d.\n", newline_position, user->fd);
    printf("server: %d of %d bytes in buffer of user %d.\n", user->inbuf, BUF_SIZE, user->fd);

    // Do nothing if the input is empty
    if (input[0] == '\0') {
        free(input);
        return 0;
    }

    // Process user input
    if (user->username == NULL) {
        add_username(socket_fd, user, input);
    } else if (strcmp(input, "-q") == 0) {
        prompt(socket_fd, user->fd, "Goodbye!\n");
        free(input);
        return -1;
    } else if (strcmp(input, "-u") == 0) {
        if (user->room == NULL) {
            list_users(socket_fd, user);
        } else {
            list_users_room(socket_fd, user);
        }
    } else if (user->room == NULL) {
        join_room(socket_fd, user, input);
    } else if (strcmp(input, "-l") == 0) {
        leave_room(socket_fd, user);
    } else {
        post(socket_fd, user, input);
    }

    free(input);
    return 0;
}


/*
Writes a prompt containing a message to a specific user.
*/
void prompt(int socket_fd, int client_fd, char *message) {
    if (write(client_fd, "> ", strlen("> ")) == -1) {
        system_error(socket_fd, "server: write");
    }
    if (write(client_fd, message, strlen(message)) == -1) {
        system_error(socket_fd, "server: write");
    }
}
