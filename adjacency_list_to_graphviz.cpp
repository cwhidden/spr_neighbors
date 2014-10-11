#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/tokenizer.hpp>
#include <boost/tuple/tuple.hpp>

using namespace std;
using namespace boost;

typedef struct vert {
	string color;
} vert;

typedef adjacency_list
< listS, vecS, undirectedS, vert, no_property>
spr_graph;

int main(int argc, char **argv) {

	int T1 = -1;
	int T2 = -1;

	if (argc >= 2) {
		T1 = atoi(argv[1]);
	}
	if (argc >= 3) {
		T2 = atoi(argv[2]);
	}

	// graph
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
		add_edge(source, target, G);
	}

	if (T1 > -1) {
		// get distance from T1
		graph_traits<spr_graph>::vertices_size_type d[num_vertices(G)];
		breadth_first_search(G, T1,
				visitor(make_bfs_visitor(record_distances(d, on_tree_edge()))));

		// color choices
		int max_distance = -1;
		for(int i = 0; i < num_vertices(G); i++) {
			if (d[i] > max_distance) {
				max_distance = i;
			}
		}

		// assign colors
		spr_graph::vertex_iterator vi, viend;
		boost::tie(vi, viend) = vertices(G);
		while (vi != viend) {
	//		if (d[*vi]
			vi++;
		}
		write_graphviz(cout, G);
	}
	else {
		write_graphviz(cout, G);
	}
	return 0;
}
