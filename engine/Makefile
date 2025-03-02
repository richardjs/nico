CFLAGS=-std=gnu17 -Wall -O3
LDFLAGS=-lm

objects=coords.o mcts.o simulate.o state.o stateio.o stateutil.o think.o tile.o

nico: $(objects)

test: $(objects)

run-test: test
	@./test

format:
	clang-format -i *.c *.h --style=WebKit

clean:
	rm -f $(objects)
	rm -f test
	rm -f nico

coords.o: coords.h state.h
mcts.o: mcts.h simulate.h state.h
simulate.o: mcts.h simulate.h state.h
state.o: coords.h state.h tile.h
stateio.o: coords.h state.h stateio.h stateutil.h tile.h
stateutil.o: coords.h state.h stateutil.h
think.o: mcts.h state.h think.h
tile.o: coords.h tile.h

# vim: set noexpandtab sts=0 ts=4:
