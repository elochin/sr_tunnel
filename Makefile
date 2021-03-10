CC=clang

DEBUG = -DDEBUG
CFLAGS= -g -Wall

all: clean sr_tunnel buffer

sr_tunnel: sr_tunnel.o util.o sr_buffer.o network.o
	${CC} ${CFLAGS} $(^) -o sr_tunnel

buffer: sr_buffer.o util.o test_buffer.c
	${CC} ${CFLAGS} $(^) -o buffer

%.o: %.c %.h
	$(CC) -o $@ -c $< $(CFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

doc: Doxyfile
	doxygen $(^)

clean:
	-rm buffer sr_tunnel *.o
