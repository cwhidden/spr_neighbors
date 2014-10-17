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
OBJS=spr_neighbors\
		 spr_dense_graph\
		 normalize\
		 1_tube\
		 adjacency_list_to_graphviz\
		 ColorGradientTest\
		 select_trees\
		 select_edges
all: $(OBJS)

spr_neighbors: spr_neighbors.cpp *.h
	$(CC) $(CFLAGS) -o spr_neighbors spr_neighbors.cpp

spr_dense_graph: spr_dense_graph.cpp *.h
	$(CC) $(CFLAGS) -o spr_dense_graph spr_dense_graph.cpp

normalize: normalize.cpp *.h
	$(CC) $(CFLAGS) -o normalize normalize.cpp

1_tube: 1_tube.cpp *.h
	$(CC) $(CFLAGS) -o 1_tube 1_tube.cpp

adjacency_list_to_graphviz: adjacency_list_to_graphviz.cpp *.h
	$(CC) $(CFLAGS) -o adjacency_list_to_graphviz adjacency_list_to_graphviz.cpp

ColorGradientTest: ColorGradientTest.cpp *.h
	$(CC) $(CFLAGS) -o ColorGradientTest ColorGradientTest.cpp

select_trees: select_trees.cpp *.h
	$(CC) $(CFLAGS) -o select_trees select_trees.cpp

select_edges: select_edges.cpp *.h
	$(CC) $(CFLAGS) -o select_edges select_edges.cpp

.PHONY: debug
.PHONY: profile
.PHONY: test

debug:
	$(CC) $(LFLAGS) $(DEBUGFLAGS) -o spr_neighbors spr_neighbors.cpp
	$(CC) $(LFLAGS) $(DEBUGFLAGS) -o 1_tube 1_tube.cpp
profile:
	$(CC) $(LFLAGS) $(DEBUGFLAGS) $(PROFILEFLAGS) -o spr_neighbors spr_neighbors.cpp
test:
	./spr_neighbors < test_trees/balanced_8
