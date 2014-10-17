#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/tokenizer.hpp>
#include <boost/tuple/tuple.hpp>
#include "ColorGradient.h"

using namespace std;
using namespace boost;

typedef adjacency_list
< listS, vecS, undirectedS, no_property, no_property>
spr_graph;

template <class Color>
class vertex_writer {
	public:
		vertex_writer(Color c) : co(c) {}
		template <class Vertex>
		void operator()(ostream &out, const Vertex& e) const {
			out << "[fillcolor=\"" << co[e] << "\"]";
		}
	private:
		Color co;
};

template <class Color>
inline vertex_writer<Color> make_vertex_writer(Color c) {
	return vertex_writer<Color>(c);
}

class edge_writer {
	public:
		edge_writer() {}
		template <class Edge>
		void operator()(ostream &out, const Edge& e) const {
		}
	private:
};


inline edge_writer make_edge_writer() {
	return edge_writer();
}

class graph_writer {
	public:
	void operator()(ostream &out) const {
		out << "outputorder=edgesfirst;" << endl;
		out << "node[label=\"\", style=\"filled\"];" << endl;
	}
};

inline graph_writer make_graph_writer() {
	return graph_writer();
}


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
		for(int i = 0; i < num_vertices(G); i++) {
			d[i] = 0;
		}
		vector<string> color = vector<string>(num_vertices(G));
		breadth_first_search(G, T1,
				visitor(make_bfs_visitor(record_distances(d, on_tree_edge()))));

		// max distance from T1
		int max_distance = -1;
		spr_graph::vertex_iterator vi, viend;
		boost::tie(vi, viend) = vertices(G);
		while (vi != viend) {
			if ((int)d[*vi] > max_distance) {
				max_distance = d[*vi];
			}
			vi++;
		}

		if (T2 <= -1) {
			// assign colors
			ColorGradient Bu = ColorGradient();
			Bu.clearGradient();
			Bu.createBu();
			boost::tie(vi, viend) = vertices(G);
			while (vi != viend) {
				float value = 1 - ((float)d[*vi] / max_distance);
				string col = Bu.getHexColorAtValue(value);
				color[*vi] = col;
				vi++;
			}
		}
		else {
			// get distance from T2
			graph_traits<spr_graph>::vertices_size_type d2[num_vertices(G)];
			for(int i = 0; i < num_vertices(G); i++) {
				d2[i] = 0;
			}
			breadth_first_search(G, vertex(T2, G),
					visitor(make_bfs_visitor(record_distances(d2, on_tree_edge())))); 
			// max distance from T2
			int max_distance2 = -1;
			spr_graph::vertex_iterator vi, viend;
			boost::tie(vi, viend) = vertices(G);
			while (vi != viend) {
				if ((int)d2[*vi] > max_distance2) {
					max_distance2 = d2[*vi];
				}
				vi++;
			}


			// assign colors
			ColorGradient Bu = ColorGradient();
			Bu.clearGradient();
			Bu.createBu();
			ColorGradient Ru = ColorGradient();
			Ru.clearGradient();
			Ru.createRu();
			boost::tie(vi, viend) = vertices(G);
			while (vi != viend) {
				// blue
				float blue_value = 1 - ((float)d[*vi] / max_distance);
				float red, green, blue;
				Bu.getColorAtValue(blue_value, red, green, blue);

				float red_value = 1 - ((float)d2[*vi] / max_distance2);
				float red2, green2, blue2;
				Ru.getColorAtValue(red_value, red2, green2, blue2);
				ColorGradient interpolate = ColorGradient();
				interpolate.addColorPoint(red, green, blue, 0.0f);
				interpolate.addColorPoint(red2, green2, blue2, 1.0f);

				float value = (float)d[*vi] / (d[*vi] + d2[*vi]);
				string col = interpolate.getHexColorAtValue(value);
				color[*vi] = col;
				vi++;
			}
		}

		// write graph
		write_graphviz(cout, G, make_vertex_writer(color), make_edge_writer(), make_graph_writer());
	}
	else {
		write_graphviz(cout, G);
	}
	return 0;
}
