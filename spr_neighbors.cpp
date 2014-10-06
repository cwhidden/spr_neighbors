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

// USAGE
string USAGE =
"spr_neighbors, version 0.0.1\n";

// FUNCTIONS

list<Node *> get_neighbors(Node *tree);
void get_neighbors(Node *n, Node *root, list<Node *> &neighbors);
void get_neighbors(Node *n, Node *new_sibling, Node *root, list<Node *> &neighbors);
void add_neighbor(Node *n, Node *new_sibling, Node *root, list<Node *> &neighbors);


// MAIN

int main(int argc, char *argv[]) {

	while (argc > 1) {
		char *arg = argv[--argc];

		if (strcmp(arg, "--help") == 0) {
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

		cout << T->str_subtree() << endl;
		// convert to integer labels
		T->labels_to_numbers(&label_map, &reverse_label_map);

		cout << T->str_subtree() << endl;

		T->normalize_order();

		cout << T->str_subtree() << endl;

		break;
	}

	// known trees
	set<string> trees = set<string>();

	// new trees this round
	list<Node *> new_trees = list<Node *>();

	// first tree
	trees.insert(T->str_subtree());
	new_trees.push_back(T);
	
	// generate a given neighborhood size (command line arg or distance-1)
	for (int i = 1; i <= DIAMETER; i++) {
		list<Node *> found_trees = list<Node *>();
		while(!new_trees.empty()) {
			Node *tree = new_trees.front();
			new_trees.pop_front();

			cout << "current_tree: " << tree->str_subtree() << endl;
			list<Node *> neighbors = get_neighbors(tree);
			list<Node*>::iterator t;
			cout << "n_size: " << neighbors.size() << endl;
			for(t = neighbors.begin(); t != neighbors.end(); t++) {
				cout << (*t)->str_subtree() << endl;
			}
		}
	}
	
	// output? just the trees? a graph?
	// just the trees for now
	
	// later, allow a pair and compute the k-tube candidates
	
	// cleanup
	T->delete_tree();

}

// get a list of a trees neighbors
list<Node *> get_neighbors(Node *tree) {
	list<Node *> neighbors = list<Node *>();
	get_neighbors(tree, tree, neighbors);
	return neighbors;
}

// consider choices of subtree source
void get_neighbors(Node *n, Node *root, list<Node *> &neighbors) {

	// recurse
	if (n->lchild() != NULL) {
		get_neighbors(n->lchild(), root, neighbors);
	}
	if (n->rchild() != NULL) {
		get_neighbors(n->rchild(), root, neighbors);
	}

	get_neighbors(n, root, root, neighbors);
}

// consider choices of subtree target
void get_neighbors(Node *n, Node *new_sibling, Node *root, list<Node *> &neighbors) {
	if (n == new_sibling) {
		return;
	}
	// recurse
	if (new_sibling->lchild() != NULL) {
		get_neighbors(n, new_sibling->lchild(), root, neighbors);
	}
	if (new_sibling->rchild() != NULL) {
		get_neighbors(n, new_sibling->rchild(), root, neighbors);
	}

	add_neighbor(n, new_sibling, root, neighbors);

}

void add_neighbor(Node *n, Node *new_sibling, Node *root, list<Node *> &neighbors) {

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

	Node *new_tree = build_tree(root->str_subtree());
	new_tree->normalize_order();
	neighbors.push_back(new_tree);
//	cout << "proposed tree: " << new_tree->str_subtree() << endl;



	n->spr(undo, which_sibling);
//	cout << "reverted: " << root->str_subtree() << endl;
//	cout << endl;
}


