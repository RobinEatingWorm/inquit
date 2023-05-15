# Inquit

Inquit (Latin for 'he says' or 'she says') is a command-line program allowing users to chat in real time. Inquit is written in C and requires a C compiler and the netcat utility.

## Server Operation

To build the server, run `make inquit` in the [src](./src/) directory. The result is shown below. The [Makefile](./src/Makefile) uses GCC as a compiler by default, but this can be changed. Likewise, the port number 54878 can also be modified should it be in use. Note that if there is no `-DPORT` flag, Inquit will default to using port 54879 (see [setup.h](./src/setup.h)).

```
$ cd src
$ make inquit
gcc -DPORT=\54878 -std=gnu99 -Wall -Werror -c inquit.c
gcc -DPORT=\54878 -std=gnu99 -Wall -Werror -c setup.c
gcc -DPORT=\54878 -std=gnu99 -Wall -Werror -c command.c
gcc -DPORT=\54878 -std=gnu99 -Wall -Werror -o inquit inquit.o setup.o command.o
```

If you do not have Make, an alternative would be to individually perform the four compilations above in the same order. The resulting executable can be run using `./inquit`. This should initialize the Inquit server and begin returning status messages.

```
$ ./inquit
server: Successfully initialized socket on file descriptor 3.
server: Blocking on select...
```

## Client Operation

Use the netcat utility to connect to the Inquit server. In the demo below, replace `<hostname>` with the name of the host for the server. You can find this name by running `hostname -f` on the device where the Inquit server is running.

```
$ nc <hostname> 54878
> Welcome to Inquit! Please enter a username.
```

When prompted, enter a username.

```
Alice
> Welcome to the lobby, Alice!
> Enter the name of the room you would like to join.
> Enter '-u' for a list of active users.
> Enter '-q' to quit.
```

Note that after a successful connection, all messages sent to the user by the Inquit server are prefixed with `> `. All messages sent by the user to the Inquit server cannot exceed 2048 characters.

### Lobby

New users are automatically placed in the lobby after choosing a username. The lobby contains all users who have not yet joined a specific room. The following commands are usable in the lobby.

* `-u`: Print a list of all active users and their current rooms.
* `-q`: Quit Inquit. You will receive a goodbye message.

**Any other input will be interpreted as the name of a room.**

In the demo below, our user Alice uses the `-u` command to before typing `main` to join a room called 'main'.

```
-u
> Users:
>  * Alice (lobby)
>  * Bob (room main)
>  * Charlie (room main)
>  * Dave (room xyzzy)
main
> Joined room main.
> Enter '-u' for a list of users in this room.
> Enter '-l' to leave or '-q' to quit.
```

### Rooms

Rooms allows you to send messages to and receive messages from all users in the same room. A room can have any name no greater than 2048 characters long (except for '-u' and '-q', as these are special commands reserved for the lobby). The following commands are usable in any room.

* `-u`: Print a list of all active users in this room.
* `-l`: Leave the room and return to the lobby.
* `-q`: Quit Inquit. You will receive a goodbye message.

**Any other input will be interpreted as a message to be sent to all users in the room.**

In the demo below, Alice begins in the lobby and joins the room 'main'. She then uses several commands and communicates with Bob and Charlie who are in the same room before quitting Inquit.

```
main
> Joined room main.
> Enter '-u' for a list of users in this room.
> Enter '-l' to leave or '-q' to quit.
-u
> Users in room main:
>  * Alice
>  * Bob
>  * Charlie
> Bob: Hi!
Hey guys!
> Alice: Hey guys!
> Charlie: Yo what's up?
-l
> Left room main. You are currently in the lobby.
> Enter the name of the room you would like to join.
> Enter '-u' for a list of active users.
> Enter '-q' to quit.
-q
> Goodbye!
```
