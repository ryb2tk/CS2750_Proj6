GCC = gcc
CFLAGS = -g

mathwait: mathwait.o
	$(GCC) $(CFLAGS) mathwait.o -o mathwait

mathwait.o: mathwait.c
	$(GCC) $(CFLAGS) -c mathwait.c

clean:
	/bin/rm mathwait.o
