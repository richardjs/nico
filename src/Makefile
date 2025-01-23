CFLAGS=-std=gnu17 -Wall -O3
LDFLAGS=-lm

objects=coords.o state.o stateio.o tile.o

test: $(objects)

run-test: test
	@./test

format:
	clang-format -i *.c *.h --style=WebKit

clean:
	rm -f $(objects)
	rm -f test

coords.o: state.h
state.o: coords.h tile.h
stateio.o: coords.h state.h
test: state.h stateio.h

# vim: set noexpandtab sts=0 ts=4:
