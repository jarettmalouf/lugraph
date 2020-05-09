CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -g3

all: Unit TSP clean

Unit: lugraph_unit.o lugraph.o
	${CC} ${CFLAGS} -o $@ $^ -lm

TSP: lugraph.o TSP6.o location.o
	${CC} ${CFLAGS} -o $@ $^ -lm

clean:
	rm -r *.o