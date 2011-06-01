/*******************************************************************************
Forest.h

Data structure for a forest of binary trees

Copyright 2009-2010 Chris Whidden
cwhidden@dal.ca
http://kiwi.cs.dal.ca/Software/RSPR
March 22, 2010
Version 1.01

This file is part of rspr.

rspr is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

rspr is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with rspr.  If not, see <http://www.gnu.org/licenses/>.

*******************************************************************************/
#ifndef INCLUDE_CSTDIO
	#define INCLUDE_CSTDIO
	#include <cstdio>
#endif
#ifndef INCLUDE_CSTDLIB
	#define INCLUDE_CSTDLIB
	#include <cstdlib>
#endif
#ifndef INCLUDE_STRING
	#define INCLUDE_STRING
	#include <string>
#endif
#ifndef INCLUDE_CSTRING
	#define INCLUDE_CSTRING
	#include <cstring>
#endif
#ifndef INCLUDE_IOSTREAM
	#define INCLUDE_IOSTREAM
	#include <iostream>
#endif
#ifndef INCLUDE_SSTREAM
	#define INCLUDE_SSTREAM
	#include <sstream>
#endif
#ifndef INCLUDE_MATH
	#define INCLUDE_MATH
	#include <math.h>
#endif
#ifndef INCLUDE_VECTOR
	#define INCLUDE_VECTOR
	#include <vector>
#endif
#ifndef INCLUDE_LIST
	#define INCLUDE_LIST
	#include <list>
#endif
#ifndef INCLUDE_DEQUE
	#define INCLUDE_DEQUE
	#include <deque>
#endif
#ifndef INCLUDE_NODE
	#define INCLUDE_NODE
	#include "Node.h"
#endif
#ifndef INCLUDE_LCA
	#define INCLUDE_LCA
	#include "LCA.h"
#endif
#ifndef INCLUDE_MAP
	#define INCLUDE_MAP
	#include <map>
#endif
#ifndef INCLUDE_LIMITS
	#define INCLUDE_LIMITS
	#include <limits>
#endif
using namespace std;

class Forest {
	public:
		vector<Node *> components;
		vector<Node *> deleted_nodes;
		bool rho;

	public:
	Forest() {
		init(vector<Node *>());
	}
	Forest(vector<Node *> components) {
		init(components);
	}
	Forest(Node *head) {
		components = vector<Node *>();
		components.push_back(new Node(*head));
		deleted_nodes = vector<Node *>();
		rho = false;
	}
	Forest(const Forest &f) {
		components = vector<Node *>(f.components.size());
		for(int i = 0; i < f.components.size(); i++) {
			//if (f.components[i] != NULL)
			components[i] = new Node(*f.components[i]);
		}
		deleted_nodes = vector<Node *>();
		rho = f.rho;
	}

	void init(vector<Node *> components) {
		this->components = components;
		deleted_nodes = vector<Node *>();
		rho = false;
	}
	~Forest() {
		for(int i = 0; i < components.size(); i++) {
			//if (components[i] != NULL)
				components[i]->delete_tree();
		}
		for(int i = 0; i < deleted_nodes.size(); i++) {
			//if (deleted_nodes[i] != NULL)
				delete deleted_nodes[i];
		}
	} 

	// swap the contents of two forests
	void swap(Forest *f) {
		vector<Node *> components_temp = this->components;
		this->components = f->components;
		f->components = components_temp;
		
		/*
		vector<Node *> deleted_nodes_temp = this->deleted_nodes;
		this->deleted_nodes = f->deleted_nodes;
		f->deleted_nodes = deleted_nodes_temp;
		*/

		bool rho_temp = this->rho;
		this->rho = f->rho;
		f->rho = rho_temp;
	}

	// print the forest
	void print_components() {
		vector<Node *>::iterator it = components.begin();
		for(it = components.begin(); it != components.end(); it++) {
			Node *root = *it;
			if (root == NULL)
				cout << "!";
			else
				root->print_subtree_hlpr();
			cout << " ";
		}
		cout << endl;
	}

	// print the components seperated by s
	void print_components(string s) {
		vector<Node *>::iterator it = components.begin();
		for(it = components.begin(); it != components.end(); it++) {
			Node *root = *it;
			if (root == NULL)
				cout << "!";
			else
				root->print_subtree_hlpr();
			cout << s;
		}
		cout << endl;
	}

	inline void add_component(Node *head) {
		components.push_back(head);
	}
	inline void add_deleted_node(Node *n) {
		deleted_nodes.push_back(n);
	}

	inline Node *get_component(int i) {
		return components[i];
	}
	inline int num_components() {
		return components.size();
	}

	// return a deque of the sibling pairs
	deque<Node *> find_sibling_pairs() {
		deque<Node *> sibling_pairs = deque<Node *>();
		vector<Node *>::iterator i;
		for(i = components.begin(); i != components.end(); i++) {
			Node *component = *i;
			vector<Node *> new_sibling_pairs = component->find_sibling_pairs();
			sibling_pairs.insert(sibling_pairs.end(), new_sibling_pairs.begin(), new_sibling_pairs.end());
		}
		return sibling_pairs;
	}

	// return a deque of the singleton leaves
	list<Node *> find_singletons() {
		list<Node *> singletons = list<Node *>();
		vector<Node *>::iterator i;
		for(i = components.begin(); i != components.end(); i++) {
			Node *component = *i;
			if (component->is_leaf()) {
				singletons.push_back(component);
			}
		}
		return singletons;
	}

	// make nodes pointed to in the forest point back
	void resync() {
		vector<Node *>::iterator i;
		for(i = components.begin(); i != components.end(); i++) {
			(*i)->resync();
		}
	}
	// clear twin pointers
	void unsync() {
		vector<Node *>::iterator i;
		for(i = components.begin(); i != components.end(); i++) {
			(*i)->unsync();
		}
	}

void labels_to_numbers(map<string, int> *label_map, map<int, string> *reverse_label_map) {
	vector<Node *>::iterator i;
	for(i = components.begin(); i != components.end(); i++) {
		(*i)->labels_to_numbers(label_map, reverse_label_map);
	}
}

void numbers_to_labels(map<int, string> *reverse_label_map) {
	vector<Node *>::iterator i;
	for(i = components.begin(); i != components.end(); i++) {
		(*i)->numbers_to_labels(reverse_label_map);
	}
}

int size() {
	return components.size();
}

bool add_rho() {
	if (rho)
		return false;
	Node *T_p = new Node("p");
	add_component(T_p);
	rho = true;
	return true;
}

bool contains_rho() {
	return rho;
}

void erase_components(int start, int end) {
	components.erase(components.begin()+start, components.begin()+end);
}
};

// Functions

vector<Node *> find_labels(vector<Node *> components);
void sync_twins(Forest *T1, Forest *T2);
void sync_interior_twins(Forest *T1, Forest *T2);
void sync_interior_twins(Node *n, LCA *twin_LCA);
void sync_interior_twins(Node *n, vector<LCA> *F2_LCAs);
vector<Node *> *find_cluster_points(Forest *f);
void find_cluster_points(Node *n, vector<Node *> *cluster_points);
void delete_and_merge_LCAs(list<Node *> *active_descendants,
		vector<LCA> *F2_LCAs, list<Node *>:: iterator node1_location,
		list<Node *>:: iterator node2_location);
void delete_and_merge_LCAs(Node *n, list<Node *> *active_descendants,
		vector<LCA> *F2_LCAs);


// return the smallest number in s
int stomini(string s) {
//	cout << "stomini" << endl;
	string number_characters = "+-0123456789";
	int i = 0;
	int min = INT_MAX;
	string current = "";
	for(int i = 0; i < s.size(); i++) {
		if (number_characters.find(s[i]) != string::npos) {
			current += s[i];
		}
		else if (current.size() > 0) {
			int num = atoi(current.c_str());
			if (num < min)
				min = num;
			current = "";
		}
	}
	if (current.size() > 0) {
		int num = atoi(current.c_str());
		if (num < min)
			min = num;
		current = "";
	}
//	cout << "returning " << min << endl;
	return min;
}


// Make the leaves of two forests point to their twin in the other tree
// Note: removes unique leaves
void sync_twins(Forest *T1, Forest *T2) {
	vector<Node *> T1_labels = vector<Node *>();
	vector<Node *> T2_labels = vector<Node *>();
	vector<Node *> T1_components = T1->components;
	vector<Node *> T2_components = T2->components;
	vector<Node *>::iterator i;
	for(i = T1_components.begin(); i != T1_components.end(); i++) {
		Node *component = *i;
		vector<Node *> unsorted_labels = component->find_leaves();
		vector<Node *>::iterator j;
		for(j = unsorted_labels.begin(); j != unsorted_labels.end(); j++) {
			Node *leaf = *j;
//			cout << "T1: " << leaf->str() << endl;
			// find smallest number contained in the label
			int number = stomini(leaf->str());
//			cout << "\t" << number << endl;
			if (number >= T1_labels.size())
				T1_labels.resize(number+1, NULL);
			T1_labels[number] = leaf;
		}
	}
	for(i = T2_components.begin(); i != T2_components.end(); i++) {
		Node *component = *i;
		vector<Node *> unsorted_labels = component->find_leaves();
		vector<Node *>::iterator j;
		for(j = unsorted_labels.begin(); j != unsorted_labels.end(); j++) {
			Node *leaf = *j;
//			cout << "T2: " << leaf->str() << endl;
			// find smallest number contained in the label
			int number = stomini(leaf->str());
//			cout << "\t" << number << endl;
			if (number >= T2_labels.size())
				T2_labels.resize(number+1, NULL);
			T2_labels[number] = leaf;
		}
	}
	int size = T1_labels.size();
	if (size > T2_labels.size())
		size = T2_labels.size();
//	cout << "Syncing Twins" << endl;
	for(int i = 0; i < size; i++) {
		Node *T1_a = T1_labels[i];
		Node *T2_a = T2_labels[i];
		if (T1_a == NULL && T2_a != NULL) {
			Node *node = T2_a->parent();
			delete T2_a;
			if (node != NULL)
				node->contract();
		}
		else if (T2_a == NULL && T1_a != NULL) {
			Node *node = T1_a->parent();
			delete T1_a;
			if (node != NULL)
				node->contract();
		}
		if (T1_a != NULL && T2_a != NULL) {
			T1_a->set_twin(T2_a);
			T2_a->set_twin(T1_a);
//			cout << T1_a->str() << endl;
		}
	}
	for(int i = size; i < T1_labels.size(); i++) {
		Node *T1_a = T1_labels[i];
		Node *node = T1_a->parent();
		delete T1_a;
		if (node != NULL)
			node->contract();
	}
	for(int i = size; i < T2_labels.size(); i++) {
		Node *T2_a = T2_labels[i];
		Node *node = T2_a->parent();
		delete T2_a;
		if (node != NULL)
			node->contract();
	}
//	cout << "Finished Syncing Twins" << endl;
	return;
}

/* make interior nodes point to the lca of their descendants in the other tree
   assumes that sync_twins has already been called
   assumes that component 1 of T1 matches with 1 of T2
      NOTE: this isn't true during the algorithm so this will need to be changed
      if we want to interleave clustering. It should be just component 1 of T1
	  matching multiple components of T2 (The first several components?)
   */
void sync_interior_twins(Forest *T1, Forest *T2) {
	Node  *root1 = T1->get_component(0);
	Node  *root2 = T2->get_component(0);
	LCA T1_LCA = LCA(root1);
	LCA T2_LCA = LCA(root2);
	sync_interior_twins(root1, &T2_LCA);
	sync_interior_twins(root2, &T1_LCA);
}

/* make interior nodes point to the LCA of their descendants in the other
	 forest if there is one unambiguous LCA
	 * This is true for a node n of T1 if all leaves that are a descendant
	 	of T1 either map to a single component of F2 or are from another
	 	component of F2 such that the root of that component maps to
	 	a descendant of n (i.e. a finished component)
   * assumes that sync_twins has already been called
	 */
void sync_interior_twins_real(Forest *T1, Forest *F2) {
	Node  *T1_root = T1->get_component(0);
	LCA T1_LCA = LCA(T1_root);
	int T1_size = T1_root->size_using_prenum();
	// roots of F2
	vector<Node *> F2_roots = vector<Node *>();
	// LCA queries for F2
	vector<LCA> F2_LCAs = vector<LCA>();
	// lists of root nodes that map to a given T1 node
	T1_root->initialize_parameter(ROOT_LCAS, list<Node *>());
	// list of active descendants
	T1_root->initialize_parameter(ACTIVE_DESCENDANTS, list<Node *>());

	// should be fine.
	for(int i = 0; i < F2->num_components(); i++) {
		F2_roots.push_back(F2->get_component(i));
		// ignore finished components
		if (F2_roots[i]->get_twin() != NULL && F2_roots[i]->get_twin()->parent() == NULL)
			continue;
		F2_LCAs.push_back(LCA(F2_roots[i]));
		// number the component
		F2_roots[i]->initialize_parameter(COMPONENT_NUMBER,i);
		// list of nodes that get deleted when a component is finished
		F2_roots[i]->initialize_parameter(REMOVABLE_DESCENDANTS,list<list<Node *>::iterator>());
		// sync the component with T1
		sync_interior_twins(F2_roots[i], &T1_LCA);
		// keep reverse pointer for the root's twin
		boost::any_cast<list<Node *> >(F2_roots[i]->get_twin()->get_parameter_ref(ROOT_LCAS))->push_back(F2_roots[i]);
	}
	sync_interior_twins(T1_root, &F2_LCAs); 
}

/* make interior nodes point to the lca of their descendants in the other
 * tree
 * assumes that sync_twins has already been called
 */
void sync_interior_twins(Node *n, LCA *twin_LCA) {
	Node *lc = n->lchild();
	Node *rc = n->rchild();
	if (lc != NULL)
		sync_interior_twins(lc, twin_LCA);
	if (rc != NULL)
		sync_interior_twins(rc, twin_LCA);
	if (lc == NULL && rc != NULL)
		n->set_twin(rc->get_twin());
	else if (lc != NULL && rc == NULL)
		n->set_twin(lc->get_twin());
	else if (lc != NULL && rc != NULL) {
		Node *twin = twin_LCA->get_lca(lc->get_twin(), rc->get_twin());
		n->set_twin(twin);
	}
}

void sync_interior_twins(Node *n, vector<LCA> *F2_LCAs) {
	Node *lc = n->lchild();
	Node *rc = n->rchild();
	list<Node *> *active_descendants = boost::any_cast<list<Node *> >(n->get_parameter_ref(ACTIVE_DESCENDANTS));
	// visit left subtree first
	if (lc != NULL)
		sync_interior_twins(lc, F2_LCAs);
	// visit right subtree first
	if (rc != NULL)
		sync_interior_twins(rc, F2_LCAs);
	if (lc == NULL && rc == NULL) {
		active_descendants->push_back(n->get_twin());
		list<Node *>::iterator node_location = active_descendants->end();
		node_location--;
			boost::any_cast<list<list<Node *>::iterator> >(n->get_twin()->get_parameter_ref(REMOVABLE_DESCENDANTS))->push_back(node_location);
	}
	// no lc so propogate up
	if (lc == NULL && rc != NULL) {
		n->set_twin(rc->get_twin());
		list<Node *> *rc_active_descendants = boost::any_cast<list<Node *> >(rc->get_parameter_ref(ACTIVE_DESCENDANTS));
		active_descendants->splice(active_descendants->end(),*rc_active_descendants);
	}
	// no rc so propogate up
	if (lc != NULL && rc == NULL) {
		n->set_twin(lc->get_twin());
		list<Node *> *lc_active_descendants = boost::any_cast<list<Node *> >(lc->get_parameter_ref(ACTIVE_DESCENDANTS));
		active_descendants->splice(active_descendants->end(),*lc_active_descendants);
	}
	else if (lc != NULL && rc == NULL)
		n->set_twin(lc->get_twin());
	// two children so put their info together
	else if (lc != NULL && rc != NULL) {
		list<Node *> *lc_active_descendants = boost::any_cast<list<Node *> >(lc->get_parameter_ref(ACTIVE_DESCENDANTS));
		list<Node *> *rc_active_descendants = boost::any_cast<list<Node *> >(rc->get_parameter_ref(ACTIVE_DESCENDANTS));
		bool done = false;
		if (lc_active_descendants->empty() || rc_active_descendants->empty())
			done = true;
		active_descendants->splice(active_descendants->end(),*lc_active_descendants);
		list<Node *>::iterator node1_location = active_descendants->end();
		node1_location--;
		list<Node *>::iterator node2_location = node1_location;
		active_descendants->splice(active_descendants->end(),*rc_active_descendants);
		node2_location++;
		/* check the intersection point to see if we have two
			leaves from the same component
		*/
		if (!done)
			delete_and_merge_LCAs(active_descendants, F2_LCAs, node1_location,
					node2_location);

		/* check to see if n is twinned by a root of F2
			 if so, then remove each leaf twinned by that component
			 and check each of the new intersection points
		*/
		list<Node *> *root_lcas = boost::any_cast<list<Node *> >(n->get_parameter_ref(ROOT_LCAS));
		while(!root_lcas->empty()) {
			Node *root_lca = root_lcas->front();
			root_lcas->pop_front();
			/* TODO: problem when n is a root
				 We don't care about this but it might mean there is a different
				 problem
				 */
			if (n->parent() != NULL)
				delete_and_merge_LCAs(root_lca, active_descendants, F2_LCAs);
		}

		/* If we have a single element in n's active descendants
			 list then set twin pointers appropriately
		*/
		if (active_descendants->size() == 1) {
			Node *twin = active_descendants->front();
			n->set_twin(twin);
		}
	}
}

/* merge two nodes from a list into their LCA if they are from
	 the same component
	 */
void delete_and_merge_LCAs(list<Node *> *active_descendants,
		vector<LCA> *F2_LCAs, list<Node *>:: iterator node1_location,
		list<Node *>:: iterator node2_location) {

//	while(active_descendants->size() > 1) {
	Node *n1 = *node1_location;
	Node *n2 = *node2_location;
	int component1 = boost::any_cast<int>(n1->get_parameter(COMPONENT_NUMBER));
	int component2 = boost::any_cast<int>(n2->get_parameter(COMPONENT_NUMBER));
	if (component1 == component2) {
		Node *lca = (*F2_LCAs)[component1].get_lca(n1,n2);
		list<Node *>::iterator lca_location =
			active_descendants->insert(node1_location,lca);
		boost::any_cast<list<list<Node *>::iterator> >(lca->get_parameter_ref(REMOVABLE_DESCENDANTS))->push_back(lca_location);
		active_descendants->erase(node1_location);
		boost::any_cast<list<list<Node *>::iterator> >(n1->get_parameter_ref(REMOVABLE_DESCENDANTS))->pop_back();
		active_descendants->erase(node2_location);
		boost::any_cast<list<list<Node *>::iterator> >(n2->get_parameter_ref(REMOVABLE_DESCENDANTS))->pop_back();
	}
//		else {
//			break;
//		}
//	}
}

/* delete each leaf from the list that is twinned with the component
	 of n. For each such deleted node, merge its predecessor
	 and successor in the list into their LCA if they are from
	 the same component (other than n's component)
	 */
void delete_and_merge_LCAs(Node *n, list<Node *>
		*active_descendants, vector<LCA> *F2_LCAs) {
	int component = boost::any_cast<int>(n->get_parameter(COMPONENT_NUMBER));
	list<list<Node *>::iterator> *removable_descendants	=
			boost::any_cast<list<list<Node *>::iterator> >(n->get_parameter_ref(REMOVABLE_DESCENDANTS));
	while (!removable_descendants->empty()) {
		list<Node *>::iterator leaf_location = removable_descendants->front();
		removable_descendants->pop_front();

		if (active_descendants->front() != *leaf_location
				&& active_descendants->back() != *leaf_location) {
			list<Node *>::iterator node1_location = leaf_location;
			list<Node *>::iterator node2_location = leaf_location;
			node1_location--;
			node2_location++;
			active_descendants->erase(leaf_location);
			int node1_component = boost::any_cast<int>((*node1_location)->get_parameter(COMPONENT_NUMBER));
			if (component != node1_component)
				delete_and_merge_LCAs(active_descendants, F2_LCAs, node1_location,
						node2_location);
		}
		else {
			active_descendants->erase(leaf_location);
		}

	}
}

vector<Node *> *find_cluster_points(Forest *F) {
	vector<Node *> *cluster_points = new vector<Node *>();
	find_cluster_points(F->get_component(0), cluster_points);
	return cluster_points;
}

// find the cluster points
void find_cluster_points(Node *n, vector<Node *> *cluster_points) {
	Node *lc = n->lchild();
	Node *rc = n->rchild();
	if (lc != NULL)
		find_cluster_points(lc, cluster_points);
	if (rc != NULL)
		find_cluster_points(rc, cluster_points);
	if (n == n->get_twin()->get_twin()
			&& n->parent() != NULL
			&& lc != NULL
			&& rc != NULL
			&& (lc != lc->get_twin()->get_twin()
				|| rc != rc->get_twin()->get_twin())) {
		cluster_points->push_back(n);
	}
}

// swap two forests
void swap(Forest **a, Forest **b) {
	(*a)->swap(*b);
}

// expand all contracted nodes
void expand_contracted_nodes(Forest *F) {
	for(int i = 0; i < F->size(); i++) {
		expand_contracted_nodes(F->get_component(i));
	}
}
