#ifndef COMMAND_H
#define COMMAND_H


#define BUF_SIZE 2048
#define LOBBY_INST "> Enter the name of the room you would like to join.\n> Enter '-u' for a list of active users.\n> Enter '-q' to quit.\n"
#define ROOM_INST "> Enter '-u' for a list of users in this room.\n> Enter '-l' to leave or '-q' to quit.\n"


typedef struct user {
    int fd;
    char *username;
    char *room;
    char *buf;
    int inbuf;
    struct user *next;
} User;


/*
Add a client with the given file descriptor to the list of users.
*/
void init_user(int socket_fd, int client_fd);


/*
Free the memory of a user.
*/
void free_user(User *user);


/*
Free the memory of all users.
*/
void free_users();


/*
Search the characters in buf for '\r' followed by '\n'. Return one plus the index of the '\n',
or -1 if no newline is found.
*/
int get_newline_double(char *buf, int inbuf);


/*
Search the characters in buf for either '\r' or '\n'. Return one plus the index of whichever
character is found, or -1 if no newline is found.
*/
int get_newline_single(char *buf, int inbuf);


/*
Add a username to a new user and prompt them to join a room.
*/
void add_username(int socket_fd, User *user, char *username);


/*
Write a list of all users.
*/
void list_users(int socket_fd, User *user);


/*
Write a list of all users in the same room as the user.
*/
void list_users_room(int socket_fd, User *user);


/*
Add a user to a room and prompt them to send messages.
*/
void join_room(int socket_fd, User *user, char *room);


/*
Remove a user from a room and prompt them to join a room.
*/
void leave_room(int socket_fd, User *user);


/*
Send a message from the user to all other users in the same room.
*/
void post(int socket_fd, User *user, char *message);


/*
Add user input to the user's buffer. If a network newline is detected, process the user's input.
Return 0 if successful or -1if the file descriptor is closed.
*/
int process_input(int socket_fd, User *user);


/*
Writes a prompt to a specific user. The prompt may optionally contain a message.
*/
void prompt(int socket_fd, int client_fd, char *message);


#endif // COMMAND_H
