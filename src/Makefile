PORT=54878
OPTIONS= -DPORT=\$(PORT) -std=gnu99 -Wall -Werror

inquit: inquit.o setup.o command.o
	gcc $(OPTIONS) -o $@ $^

%.o: %.c setup.h command.h
	gcc $(OPTIONS) -c $<

.PHONY: clean
clean:
	rm inquit *.o
