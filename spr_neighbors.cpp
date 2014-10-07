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

using namespace std;

// OPTIONS

int DIAMETER = 1;
bool SIZE_ONLY = false;

// USAGE
string USAGE =
"spr_neighbors, version 0.0.1\n";

// FUNCTIONS

list<Node *> get_neighbors(Node *tree, set<string> &known_trees);
void get_neighbors(Node *n, Node *root, list<Node *> &neighbors, set<string> &known_trees);
void get_neighbors(Node *n, Node *new_sibling, Node *root, list<Node *> &neighbors, set<string> &known_trees);
void add_neighbor(Node *n, Node *new_sibling, Node *root, list<Node *> &neighbors, set<string> &known_trees);


// MAIN

int main(int argc, char *argv[]) {
	int max_args = argc-1;
	while (argc > 1) {
		char *arg = argv[--argc];

		if (strcmp(arg, "--size_only") == 0) {
			SIZE_ONLY = true;
		}
		else if (strcmp(arg, "-k") == 0) {
			if (max_args > argc) {
				char *arg2 = argv[argc+1];
				if (arg2[0] != '-') {
					DIAMETER = atoi(arg2);
				}
			}
		}
		else if (strcmp(arg, "--help") == 0) {
			cout << USAGE;
			return 0;
		}
	}

	// initialize random number generator
	srand((unsigned(time(0))));

	// label maps to allow string labels
	map<string, int> label_map= map<string, int>();
	map<int, string> reverse_label_map = map<int, string>();
	
	// start tree
	Node *T;
	string name = "";

	string T_line;

	// read in a single tree
	while (getline(cin, T_line)) {

		size_t loc = T_line.find_first_of("(");

		if (loc == string::npos) {
			continue;
		}
		if (loc != 0) {
			name = T_line.substr(0,loc);
			T_line.erase(0,loc);
		}
		T = build_tree(T_line);
		T->preorder_number();
		T->edge_preorder_interval();

//		cout << T->str_subtree() << endl;
		// convert to integer labels
		T->labels_to_numbers(&label_map, &reverse_label_map);

//		cout << T->str_subtree() << endl;

		T->normalize_order();

//		cout << T->str_subtree() << endl;

//		cout << endl;

		break;
	}

	// TODO: vector of neighbourhood distances?
	// known trees
	set<string> known_trees = set<string>();

	// new trees this round
	list<Node *> new_trees = list<Node *>();
	list<Node *> next_trees = list<Node *>();


	// first tree
	known_trees.insert(T->str_subtree());
	new_trees.push_back(T);
	
	// generate a given neighborhood size (command line arg or distance-1)
	for (int i = 1; i <= DIAMETER; i++) {
		list<Node *> found_trees = list<Node *>();
		while(!new_trees.empty()) {
			Node *tree = new_trees.front();
			new_trees.pop_front();

//			cout << "current_tree: " << tree->str_subtree() << endl;
			list<Node *> neighbors = get_neighbors(tree, known_trees);
			list<Node*>::iterator t;
//			cout << "n_size: " << neighbors.size() << endl;
			for(t = neighbors.begin(); t != neighbors.end(); t++) {
				// add to known trees and next_trees
				string name = (*t)->str_subtree();
				known_trees.insert(name);
				next_trees.push_back(*t);
			}
			// cleanup
			tree->delete_tree();
		}
		new_trees = next_trees;
		next_trees = list<Node *>();
	}
	// cleanup
//	cout << new_trees.size() << endl;
	while(!new_trees.empty()) {
		Node *tree = new_trees.front();
		new_trees.pop_front();
		tree->delete_tree();
	}

	// output
	if (SIZE_ONLY) {
		cout << known_trees.size() << endl;
	}
	else {
		set<string>::iterator t;
		for(t = known_trees.begin(); t != known_trees.end(); t++) {
			// hacky - do this better if necessary
			Node *T = build_tree(*t);
			T->numbers_to_labels(&reverse_label_map);
			cout << T->str_subtree() << endl;
			T->delete_tree();
		}
	}
	
	// output? just the trees? a graph?
	// just the trees for now
	
	// later, allow a pair and compute the k-tube candidates
	
}

// get a list of a trees neighbors
list<Node *> get_neighbors(Node *tree, set<string> &known_trees) {
	list<Node *> neighbors = list<Node *>();
	get_neighbors(tree, tree, neighbors, known_trees);
	return neighbors;
}

// consider choices of subtree source
void get_neighbors(Node *n, Node *root, list<Node *> &neighbors, set<string> &known_trees) {

	// recurse
	if (n->lchild() != NULL) {
		get_neighbors(n->lchild(), root, neighbors, known_trees);
	}
	if (n->rchild() != NULL) {
		get_neighbors(n->rchild(), root, neighbors, known_trees);
	}

	get_neighbors(n, root, root, neighbors, known_trees);
}

// consider choices of subtree target
void get_neighbors(Node *n, Node *new_sibling, Node *root, list<Node *> &neighbors, set<string> &known_trees) {
	if (n == new_sibling) {
		return;
	}
	// recurse
	if (new_sibling->lchild() != NULL) {
		get_neighbors(n, new_sibling->lchild(), root, neighbors, known_trees);
	}
	if (new_sibling->rchild() != NULL) {
		get_neighbors(n, new_sibling->rchild(), root, neighbors, known_trees);
	}

	add_neighbor(n, new_sibling, root, neighbors, known_trees);

}

void add_neighbor(Node *n, Node *new_sibling, Node *root, list<Node *> &neighbors, set<string> &known_trees) {

	// check for obvious duplicates
	if (n->parent() != NULL &&
			(new_sibling == n->parent())) {
	// cout << "rule 1" << endl;
		return;
	}
	if (n->parent() != NULL &&
			new_sibling->parent() != NULL &&
			n->parent()->parent() == new_sibling->parent()) {
//		cout << "rule 2" << endl;
		return;
	}
	if (new_sibling == n->get_sibling()) {
//		cout << "rule 3" << endl;
		return;
	}
//		cout << "foo3" << endl;
//	cout << "foo4" << endl;
	if (new_sibling == n) {
//		cout << "rule 4" << endl;
		return;
	}
	Node *old_sibling = n->get_sibling();
	//if (new_sibling != old_sibling)
	//
	int which_sibling = 0;
//	cout << "original: " << root->str_subtree() << endl;
	Node *undo = n->spr(new_sibling, which_sibling);

	string name = root->str_subtree();
	// quick check for obvious duplicate
	set<string>::iterator x = known_trees.find(name);
	if (x == known_trees.end()) {
		Node *new_tree = build_tree(name);
		// normalize and check again
		new_tree->normalize_order();
		name = new_tree->str_subtree();
		x = known_trees.find(name);
		if (x == known_trees.end()) {
			known_trees.insert(name);
			neighbors.push_back(new_tree);
		}
		else {
			new_tree->delete_tree();
		}
	}
//	cout << "proposed tree: " << new_tree->str_subtree() << endl;



	n->spr(undo, which_sibling);
//	cout << "reverted: " << root->str_subtree() << endl;
//	cout << endl;
}


