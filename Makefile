PORT=54878
OPTIONS= -DPORT=\$(PORT) -std=gnu99 -Wall -Werror

inquit: inquit.o setup.o command.o
	gcc $(OPTIONS) -o inquit inquit.o setup.o command.o

inquit.o: inquit.c setup.h command.h
	gcc $(OPTIONS) -c inquit.c

setup.o: setup.c setup.h command.h
	gcc $(OPTIONS) -c setup.c

command.o: command.c setup.h command.h
	gcc $(OPTIONS) -c command.c

clean:
	rm inquit *.o
