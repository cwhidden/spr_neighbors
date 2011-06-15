/*******************************************************************************
ClusterInstance.h


Copyright 2011 Chris Whidden
cwhidden@dal.ca
http://kiwi.cs.dal.ca/Software/RSPR
June 8, 2011
Version 1.02

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
#ifndef INCLUDE_CLUSTERINSTANCE

#define INCLUDE_CLUSTERINSTANCE
#include "Forest.h"
#include "Node.h"

using namespace std;

class ClusterInstance {
	public:
		Forest *F1;
		Forest *F2;
		Node *F1_cluster_node;
		Node *F2_cluster_node;

	public:
	ClusterInstance(){
		init(NULL, NULL, NULL, NULL);
	}
	ClusterInstance(Forest *f1, Forest *f2, Node *f1_cluster_node,
			Node *f2_cluster_node) {
		init (f1, f2, f1_cluster_node, f2_cluster_node);
	}
	ClusterInstance(const ClusterInstance &c) {
		init(c.F1,c.F2,c.F1_cluster_node,c.F2_cluster_node);
	}

	void init(Forest *f1, Forest *f2, Node *f1_cluster_node,
			Node *f2_cluster_node) {
		F1 = f1;
		F2 = f2;
		F1_cluster_node = f1_cluster_node;
		F2_cluster_node = f2_cluster_node;
	}

	~ClusterInstance() {
	} 

	bool is_original() {
		if (F1_cluster_node == NULL && F2_cluster_node == NULL)
			return true;
		else
			return false;
	}

	void join_cluster(Forest *original_F1, Forest *original_F2) {
		int start = 0;
		if (F1->contains_rho()) {
			F1_cluster_node->contract();
		}
		else {
			F1_cluster_node->add_child(F1->get_component(0));
			start = 1;
		}
		// should we add these to a finished_components or something?
		for(int i = start; i < F1->num_components(); i++) {
			if (F1->get_component(i)->str() != "p")
				original_F1->add_component(F1->get_component(i));
		}

		int skip = -1;
		if (F2->contains_rho()) {
			F2_cluster_node->contract();
		}
		else if (F2_cluster_node != NULL) {
			F2_cluster_node->add_child(F1->get_component(0)->get_twin());
			skip = boost::any_cast<int>(F1->get_component(0)->get_twin()->
					get_parameter(COMPONENT_NUMBER)); 
		}
		// should we add these to a finished_components or something?
		for(int i = 0; i < F2->num_components(); i++) {
			if (i != skip && F2->get_component(i)->str() != "p")
				original_F2->add_component(F2->get_component(i));
		}

		F1->erase_components();
		F2->erase_components();
//		cout << "Joined" << endl;
//		cout << F1_cluster_node->str_subtree() << endl;
//		cout << F2_cluster_node->str_subtree() << endl;

	}

};

void cluster_reduction_find_components(Node *n,
				vector<bool> *F2_cluster_copy_components,
				vector<bool> *old_F2_keep_components,
				int cluster_component_number) {
	Node *lc = n->lchild();
	Node *rc = n->rchild();
	if (lc != NULL)
		cluster_reduction_find_components(lc, F2_cluster_copy_components,
				old_F2_keep_components, cluster_component_number);
	if (rc != NULL)
		cluster_reduction_find_components(rc, F2_cluster_copy_components,
				old_F2_keep_components, cluster_component_number);
	if (lc == NULL && rc == NULL) {
		int cnumber = boost::any_cast<int>(
				n->get_twin()->get_parameter(COMPONENT_NUMBER)); 
		if (cnumber != cluster_component_number) {
			(*F2_cluster_copy_components)[cnumber] = true;
			(*old_F2_keep_components)[cnumber] = false;
		}
	}
}

list<ClusterInstance> cluster_reduction(Forest *old_F1, Forest *old_F2,
		list<Node *> *cluster_points) {
	list<ClusterInstance> clusters = list<ClusterInstance>();
	vector<bool> old_F2_keep_components =
		vector<bool>(old_F2->num_components(), true);
	for(list<Node *>::iterator i = cluster_points->begin();
			i != cluster_points->end(); i++) {
		// Cluster F1
		Node *F1_root_node = *i;
		Node *F1_cluster_node = F1_root_node->parent();
		Node *p = F1_cluster_node;
//		while (p->parent() != NULL)
//			p = p->parent();
//		cout << "root address=" << &(*p) << endl;

		F1_root_node->cut_parent();
		vector<Node *> cluster_F1_components = vector<Node *>();
		cluster_F1_components.push_back(F1_root_node);
		Forest *F1 = new Forest(cluster_F1_components);

		// Cluster F2
		Node *F2_root_node = F1_root_node->get_twin();
		Node *F2_cluster_node = F2_root_node->parent();
		vector<bool> F2_cluster_copy_components =
			vector<bool>(old_F2->num_components(), false);
		int cnumber = boost::any_cast<int>(
				F2_root_node->get_parameter(COMPONENT_NUMBER)); 
		if (F2_root_node->parent() != NULL) {
			F2_root_node->cut_parent();
		}
		else {
			old_F2_keep_components[cnumber] = false;
		}
		cluster_reduction_find_components(F1_root_node,
				&F2_cluster_copy_components, &old_F2_keep_components, cnumber);
		vector<Node *> cluster_F2_components = vector<Node *>();
		cluster_F2_components.push_back(F2_root_node);
		for(int i = 0; i < F2_cluster_copy_components.size(); i++) {
			if (F2_cluster_copy_components[i] == true)
				cluster_F2_components.push_back(old_F2->get_component(i));
		}
		Forest *F2 = new Forest(cluster_F2_components);
		clusters.push_back(ClusterInstance(F1, F2, F1_cluster_node,
					F2_cluster_node));

//		for(int i = 0; i < F2_cluster_copy_components.size(); i++) {
//			if (F2_cluster_copy_components[i] == true) {
//				cluster_F2_components.push_back(
//						(old_F2->get_component(i)));
//				cout << "true " ;
//			}
//			else
//				cout << "false " ;
//		}
//		cout << endl;
	}
	// remove any clustered components from old_F2
	vector<Node *> old_F2_remaining_components = vector<Node *>();
//	cout << "size=" << old_F2->num_components() << endl;
	for(int i = 0; i < old_F2_keep_components.size(); i++) {
		if (old_F2_keep_components[i] == true) {
			old_F2_remaining_components.push_back(
					(old_F2->get_component(i)));
			cout << "true " ;
		}
		else
			cout << "false " ;
	}
	cout << endl;
	cout << endl;
	Forest *replace_old_F2 = new Forest(old_F2_remaining_components);
	replace_old_F2->swap(old_F2);
	replace_old_F2->erase_components();
	delete replace_old_F2;


	clusters.push_back(ClusterInstance(old_F1, old_F2, NULL, NULL));

	return clusters;

}


#endif
