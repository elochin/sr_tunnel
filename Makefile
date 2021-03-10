CC=clang

DEBUG = -DDEBUG
CFLAGS= -g -Wall -I ./include

all: bin clean sr_tunnel buffer

sr_tunnel: sr_tunnel.o util.o sr_buffer.o network.o
	${CC} ${CFLAGS} $(patsubst %,./bin/%,$(^)) -o bin/$@

buffer: sr_buffer.o util.o test_buffer.o
	${CC} ${CFLAGS} $(patsubst %,./bin/%,$(^)) -o bin/buffer

%.o: src/%.c include/%.h
	$(CC) -o bin/$@ -I include -c $< $(CFLAGS)

%.o: src/%.c
	$(CC) -o bin/$@ -I include -c $< $(CFLAGS)

doc: Doxyfile
	doxygen $(^)

bin:
	-mkdir -p bin

clean:
	-rm bin/buffer bin/sr_tunnel bin/*.o
