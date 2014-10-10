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

using namespace std;

// OPTIONS

string USAGE =
"normalize, version 0.0.1\n";

// MAIN

int main(int argc, char *argv[]) {
	int max_args = argc-1;
	while (argc > 1) {
		char *arg = argv[--argc];

		if (strcmp(arg, "--help") == 0) {
			cout << USAGE;
			return 0;
		}
	}

	// label maps to allow string labels
	map<string, int> label_map= map<string, int>();
	map<int, string> reverse_label_map = map<int, string>();
	
	string T_line;

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
		// normalize tree and print
		Node *T = build_tree(T_line);
		T->labels_to_numbers(&label_map, &reverse_label_map);
		T->normalize_order(&reverse_label_map);
		T->numbers_to_labels(&reverse_label_map);
		cout << T->str_subtree() << endl;
		T->delete_tree();
	}
}
