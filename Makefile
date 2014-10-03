CC=g++
CC64=CC
CFLAGS=-O2 -std=c++0x -march=native
OMPFLAGS=-fopenmp
C64FLAGS=$(CFLAGS)
BOOST_GRAPH=-lboost_graph-mt
BOOST_ANY=-L/lib/libboost*
LFLAGS=#$(BOOST_GRAPH) $(BOOST_ANY)
DEBUGFLAGS=-g -O0 -std=c++0x
PROFILEFLAGS=-pg
OBJS=spr_neighbors
all: $(OBJS)

spr_neighbors: spr_neighbors.cpp *.h
	$(CC) $(CFLAGS) -o spr_neighbors spr_neighbors.cpp

.PHONY: debug
.PHONY: profile
.PHONY: test

debug:
	$(CC) $(LFLAGS) $(DEBUGFLAGS) -o spr_neighbors spr_neighbors.cpp
profile:
	$(CC) $(LFLAGS) $(DEBUGFLAGS) $(PROFILEFLAGS) -o spr_neighbors spr_neighbors.cpp
test:
	./spr_neighbors < test_trees/balanced_8
