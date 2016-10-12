CC = gcc

LFLAGS = -g -W -Wall -Wmissing-declarations -Wmissing-prototypes -Wredundant-decls -Wshadow -Wbad-function-cast -Wcast-qual -o

CFLAGS = -g -W -Wall -Wmissing-declarations -Wmissing-prototypes -Wredundant-decls -Wshadow -Wbad-function-cast -Wcast-qual

LIBS = -lm

SRC=emitter.c receiver.c medium.c
EXEC=$(SRC:.c=)

all : $(EXEC)

% : %.c socket.c fifo.c timer.c
	$(CC) $(LFLAGS) $@ $^ $(LIBS)

clean:
	rm -f *.o

clear: clean
	rm -f $(EXEC)

