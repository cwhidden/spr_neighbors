#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/tokenizer.hpp>
#include <boost/tuple/tuple.hpp>

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
	int T1 = atoi(argv[2]);
	int T2 = atoi(argv[3]);
	
	// graph
	spr_graph G;
	
	// k_tube membership
	vector<bool> k_tube;
	
	
	
	// Property accessors
	//property_map<spr_graph, k_tube_t>::type
	//	k_tube = get(k_tube, G);
	
	// read in graph to a boost graph
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
	
	// initialize k_tube membership
	k_tube = vector<bool>(num_vertices(G), false);
	
	// output graph
	//write_graphviz(cout, G);
	//
	
	// initial k-tube membership
	k_tube[T1] = true;
	k_tube[T2] = true;
	
	spr_graph::adjacency_iterator vi, vend;
	boost::tie(vi, vend) = adjacent_vertices(T1, G);
	while(vi != vend) {
		k_tube[*vi] = true;
		vi++;
	}
	boost::tie(vi, vend) = adjacent_vertices(T2, G);
	while(vi != vend) {
		k_tube[*vi] = true;
		vi++;
	}
	
	
	// for each T1 neighbor
	boost::tie(vi, vend) = adjacent_vertices(T1, G);
	while(vi != vend) {
		cout << *vi << endl;

		// found list to avoid duplicates
		vector<bool> found = vector<bool>(num_vertices(G));

		graph_traits<spr_graph>::vertices_size_type d[num_vertices(G)];
		// get shortest path lengths
		breadth_first_search(G, *vi,
				visitor(make_bfs_visitor(record_distances(d, on_tree_edge()))));
		// use list of backtrackers, initially each T2 neighbor
		list<int> backtrack = list<int>();
		spr_graph::adjacency_iterator vj, vjend;
		boost::tie(vj, vjend) = adjacent_vertices(T2, G);
		while (vj != vjend) {
			backtrack.push_back(*vj);
			found[*vj] = true;
			vj++;
		}


		// backtrack and put in k-tube
		while(!backtrack.empty()) {
			int vk = backtrack.front();
			backtrack.pop_front();
			// put in k_tube
			k_tube[vk] = true;
			found[vk] = true;

			// get neighbors that are closer to vi but have not been seen yet
			boost::tie(vj, vjend) = adjacent_vertices(vk, G);
			while (vj != vjend) {
				if (!found[*vj] && d[*vj] < d[vk]) {
					found[*vj] = true;
					backtrack.push_back(*vj);
				}
				vj++;
			}
			
		}
			
		vi++;
	}
	
	// output sorted list of nums in k-tube
	spr_graph::vertex_iterator vl, vlend;
	boost::tie(vl, vlend) = vertices(G);
	while (vl != vlend) {
		cout << *vl << ": " << k_tube[*vl] << endl;
		vl++;
	}
}
