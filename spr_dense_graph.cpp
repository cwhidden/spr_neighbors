// INCLUDES
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <climits>
#include <vector>
#include <map>
#include <utility>
#include <algorithm>
#include <list>
#include <time.h>
#include "rspr.h"

#include "Forest.h"
#include "ClusterForest.h"
#include "LCA.h"
#include "ClusterInstance.h"
#include "UndoMachine.h"
#include "lgt.h"
#include "sparse_counts.h"
#include "node_glom.h"
#include "spr_neighbors.h"
#include "nni_neighbors.h"

using namespace std;

// OPTIONS
bool NNI_ONLY = false;

// USAGE
string USAGE =
"spr_dense_graph, version 0.0.1\n";

// MAIN

int main(int argc, char *argv[]) {
	int max_args = argc-1;
	while (argc > 1) {
		char *arg = argv[--argc];

		if (strcmp(arg, "--nni") == 0) {
			NNI_ONLY = true;
		}
		if (strcmp(arg, "--help") == 0) {
			cout << USAGE;
			return 0;
		}
	}

	// label maps to allow string labels
	map<string, int> label_map= map<string, int>();
	map<int, string> reverse_label_map = map<int, string>();
	
	string T_line;
	map<string, int> trees = map<string, int>();
	int num_trees = 0;

	// read in trees
	while (getline(cin, T_line)) {
		string name = "";
		size_t loc = T_line.find_first_of("(");

		if (loc == string::npos) {
			continue;
		}
		if (loc != 0) {
			name = T_line.substr(0,loc);
			T_line.erase(0,loc);
		}
		// normalize tree and insert into map
		Node *T = build_tree(T_line);
		T->labels_to_numbers(&label_map, &reverse_label_map);
		T->normalize_order();
		trees.insert(make_pair(T->str_subtree(),num_trees));
		num_trees++;
	}

	list<pair<int,int>> adjacency_list = list<pair<int,int>>();
	map<string, int>::iterator t; 
	for(t = trees.begin(); t != trees.end(); t++) {
		Node *T = build_tree(t->first);
		int num = t->second;
//		cout << num << ": " << T->str_subtree() << endl;
		list<Node *> neighbors;
		if (NNI_ONLY) {
			neighbors = get_nni_neighbors(T);
		}
		else {
			neighbors = get_neighbors(T);
		}
		list<Node *>::iterator n;
		for(n = neighbors.begin(); n != neighbors.end(); n++) {
			string name2 = (*n)->str_subtree();
			map<string, int>::iterator t2 = trees.find(name2);
			if (t2 != trees.end()) {
				int num2 = t2->second;
				if (num < num2) {
					adjacency_list.push_back(make_pair(num, num2));
				}
			}
			(*n)->delete_tree();
		}



		// cleanup
		T->delete_tree();

	}

	adjacency_list.sort();

	// output adjacency list
	list<pair<int,int>>::iterator p;
	for(p = adjacency_list.begin(); p != adjacency_list.end(); p++) {
		cout << p->first << "," << p->second << endl;
	}
}

