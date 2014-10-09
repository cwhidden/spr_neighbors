#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/tokenizer.hpp>

using namespace std;
using namespace boost;

typedef boost::adjacency_list
< listS, vecS, undirectedS, no_property, no_property>
spr_graph;

int main(int argc, char **argv) {

// T1, T2, k as arguments
if (argc < 4) {
	cout << "usage: 1_tube.cpp <k> <T1> <T2> < adjacency_list" << endl;
}

int k = atoi(argv[1]);
int T1 = atoi(argv[2]); // TODO: may need to offset by 1
int T2 = atoi(argv[3]); // TODO: may need to offset by 1

spr_graph G;

string line;
char_separator<char> sep(", ");
while (getline(cin, line)) {
	tokenizer<char_separator<char> > tokens(line, sep);
	tokenizer<char_separator<char> >::iterator t = tokens.begin();
	if (t == tokens.end()) {
		continue;
	}
	int source = atoi((*t).c_str());
	t++;
	if (t == tokens.end()) {
		continue;
	}
	int target = atoi((*t).c_str());
	cout << source << ", " << target << endl; 
	add_edge(source, target, G);
}

// output graph
//write_graphviz(cout, G);




}



// read in graph
// put in a boost::graph

// careful: check if graph is 1-numbered or 0-numbered

// need list of T1 and T2 neighbors (or just get when necessary?)

// decorate vertices with k-tube membership (in or out)

// for each T1 neighbor
	// get shortest path lengths
	// for each T2 neighbor
	// use list of backtrackers, initially each T2 neighbor
		// backtrack and put in k-tube

// output sorted list of nums in k-tube
