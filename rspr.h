/*******************************************************************************
rspr.h

Calculate approximate and exact Subtree Prune and Regraft (rSPR)
distances and the associated maximum agreement forests (MAFs) between pairs
of rooted binary trees.
Supports arbitrary labels. See the
README for more information.

Copyright 2009-2011 Chris Whidden
whidden@cs.dal.ca
http://kiwi.cs.dal.ca/Software/RSPR
August 19, 2011
Version 1.

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

//#define DEBUG 1
//#define DEBUG_CONTRACTED
//#define DEBUG_APPROX 1
//#define DEBUG_CLUSTERS 1
//#define DEBUG_SYNC 1
#define MAX_SPR 1000

#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <climits>
#include <vector>
#include <map>
#include <unordered_map>
#include <list>
#include "Forest.h"
#include "ClusterForest.h"
#include "LCA.h"
#include "ClusterInstance.h"
#include "UndoMachine.h"

using namespace std;

// note: not using undo
int rSPR_3_approx_hlpr(Forest *T1, Forest *T2, list<Node *> *singletons,
		list<Node *> *sibling_pairs);
int rSPR_3_approx(Forest *T1, Forest *T2);
int rSPR_worse_3_approx_hlpr(Forest *T1, Forest *T2, list<Node *> *singletons, list<Node *> *sibling_pairs, Forest **F1, Forest **F2, bool save_forests);
int rSPR_worse_3_approx(Forest *T1, Forest *T2, bool sync);
int rSPR_worse_3_approx(Forest *T1, Forest *T2);
int rSPR_branch_and_bound(Forest *T1, Forest *T2);
int rSPR_branch_and_bound(Forest *T1, Forest *T2, int k);
int rSPR_branch_and_bound_range(Forest *T1, Forest *T2, int end_k);
int rSPR_branch_and_bound_range(Forest *T1, Forest *T2, int start_k,
		int end_k);
int rSPR_branch_and_bound_hlpr(Forest *T1, Forest *T2, int k,
		list<Node *> *sibling_pairs, list<Node *> *singletons, bool cut_b_only,
		list<pair<Forest,Forest> > *AFs);

bool BB = false;
bool APPROX_CHECK_COMPONENT = false;
bool CUT_ONE_B = false;
bool CUT_ALL_B = false;
bool CUT_AC_SEPARATE_COMPONENTS = false;
bool CUT_ONE_AB = false;
bool CLUSTER_REDUCTION = false;
bool PREFER_RHO = false;
bool MAIN_CALL = true;
bool MEMOIZE = false;
bool ALL_MAFS = false;
int NUM_CLUSTERS = 0;
int MAX_CLUSTERS = -1;

	class ProblemSolution {
		public:
		string T1;
		string T2;
		int k;

		ProblemSolution(Forest *t1, Forest *t2, int new_k) {
			T1 = t1->str();
			T2 = t2->str();
			k = new_k;
		}
	};

	unordered_map<string, ProblemSolution> memoized_clusters = unordered_map<string, ProblemSolution>();

/* rSPR_3_approx
 * Calculate an approximate maximum agreement forest and SPR distance
 * RETURN At most 3 times the rSPR distance
 * NOTE: destructive. The computed forests replace T1 and T2.
 */
int rSPR_3_approx(Forest *T1, Forest *T2) {
	// find sibling pairs of T1
	// match up nodes of T1 and T2
	sync_twins(T1, T2);
	// find singletons of T2
	list<Node *> *sibling_pairs = T1->find_sibling_pairs();
	list<Node *> singletons = T2->find_singletons();
	int ans = rSPR_3_approx_hlpr(T1, T2, &singletons, sibling_pairs);
	delete sibling_pairs;
	return ans;
}

// rSPR_3_approx recursive helper function
int rSPR_3_approx_hlpr(Forest *T1, Forest *T2, list<Node *> *singletons,
		list<Node *> *sibling_pairs) {
	int num_cut = 0;
	while(!singletons->empty() || !sibling_pairs->empty()) {
		// Case 1 - Remove singletons
		while(!singletons->empty()) {

			Node *T2_a = singletons->back();
			singletons->pop_back();
			// find twin in T1
			Node *T1_a = T2_a->get_twin();
			// if this is in the first component of T_2 then
			// it is not really a singleton.
			if (T2_a == T2->get_component(0))
				continue;

			Node *T1_a_parent = T1_a->parent();
			if (T1_a_parent == NULL)
				continue;
			bool potential_new_sibling_pair = T1_a_parent->is_sibling_pair();
			// cut the edge above T1_a
			T1_a->cut_parent();
			T1->add_component(T1_a);
			if (T1_a->get_sibling_pair_status() > 0)
				T1_a->clear_sibling_pair(sibling_pairs);
			//delete(T1_a);

			Node *node = T1_a_parent->contract();
			if (potential_new_sibling_pair && node->is_sibling_pair()){
				node->rchild()->add_to_front_sibling_pairs(sibling_pairs, 2);
				node->lchild()->add_to_front_sibling_pairs(sibling_pairs, 1);
			}

		}
		if(!sibling_pairs->empty()) {
			Node *T1_a = sibling_pairs->back();
			sibling_pairs->pop_back();
			Node *T1_c = sibling_pairs->back();
			sibling_pairs->pop_back();
			T1_a->clear_sibling_pair_status();
			T1_c->clear_sibling_pair_status();
			if (T1_a->parent() == NULL || T1_a->parent() != T1_c->parent()) {
				continue;
			}
			Node *T1_ac = T1_a->parent();
			// lookup in T2 and determine the case
			Node *T2_a = T1_a->get_twin();
			Node *T2_c = T1_c->get_twin();

			// Case 2 - Contract identical sibling pair
			if (T2_a->parent() != NULL && T2_a->parent() == T2_c->parent()) {
				Node *T2_ac = T2_a->parent();
				T1_ac->contract_sibling_pair();
				T2_ac->contract_sibling_pair();
				T1_ac->set_twin(T2_ac);
				T2_ac->set_twin(T1_ac);
				T1->add_deleted_node(T1_a);
				T1->add_deleted_node(T1_c);
				T2->add_deleted_node(T2_a);
				T2->add_deleted_node(T2_c);

				// check if T2_ac is a singleton
				if (T2_ac->is_singleton() && !T1_ac->is_singleton() && T2_ac != T2->get_component(0))
					singletons->push_back(T2_ac);
				// check if T1_ac is part of a sibling pair
				if (T1_ac->parent() != NULL && T1_ac->parent()->is_sibling_pair()) {
					T1_ac->parent()->lchild()->add_to_sibling_pairs(sibling_pairs, 1);
					T1_ac->parent()->rchild()->add_to_sibling_pairs(sibling_pairs, 2);
				}
			}
			// Case 3
			else {
				
				//  ensure T2_a is below T2_c
				if (T2_a->get_depth() < T2_c->get_depth()) {
					swap(&T1_a, &T1_c);
					swap(&T2_a, &T2_c);
				}
				else if (T2_a->get_depth() == T2_c->get_depth()) {
					if (T2_a->parent() && T2_c->parent() &&
							(T2_a->parent()->get_depth() <
							T2_c->parent()->get_depth())) {
					swap(&T1_a, &T1_c);
					swap(&T2_a, &T2_c);
					}
				}

				// get T2_b
				Node *T2_ab = T2_a->parent();
				Node *T2_b = T2_ab->rchild();
				if (T2_b == T2_a)
					T2_b = T2_ab->lchild();
				// cut T1_a, T1_c, T2_a, T2_b, T2_c

				bool cut_b_only = false;
				if (T2_a->parent() != NULL && T2_a->parent()->parent() != NULL && T2_a->parent()->parent() == T2_c->parent()) {
					cut_b_only = true;
					T1_a->add_to_sibling_pairs(sibling_pairs,1);
					T1_c->add_to_sibling_pairs(sibling_pairs,2);
				}

				if (!cut_b_only) {
					T1_a->cut_parent();
					T1_c->cut_parent();
					// contract parents
					Node *node = T1_ac->contract();
					// check for T1_ac sibling pair
					if (node && node->is_sibling_pair()){
						node->lchild()->add_to_sibling_pairs(sibling_pairs,1);
						node->rchild()->add_to_sibling_pairs(sibling_pairs,2);
					}
				}

				bool same_component = true;
				if (APPROX_CHECK_COMPONENT)
					same_component = (T2_a->find_root() == T2_c->find_root());

				if (!cut_b_only) {
					T2_a->cut_parent();
					num_cut++;
				}
				bool cut_b = false;
				if (same_component && T2_ab->parent() != NULL) {
					T2_b->cut_parent();
					num_cut++;
					cut_b = true;
				}
				// T2_b will move up after contraction
				else {
					T2_b = T2_b->parent();
				}
				// check for T2 parents as singletons
				Node *node = T2_ab->contract();
				if (node != NULL && node->is_singleton()
						&& node != T2->get_component(0))
					singletons->push_back(node);

				// if T2_c is gone then its replacement is in singleton list
				// contract might delete old T2_c, see where it is
				bool add_T2_c = true;
				T2_c = T1_c->get_twin();
				// ignore T2_c if it is a singleton
				if (T2_c != node && T2_c->parent() != NULL && !cut_b_only) {

					Node *T2_c_parent = T2_c->parent();
					T2_c->cut_parent();
					num_cut++;
					node = T2_c_parent->contract();
					if (node != NULL && node->is_singleton()
							&& node != T2->get_component(0))
						singletons->push_back(node);
				}
				else {
					add_T2_c = false;
				}

				
				if (!cut_b_only)
					T1->add_component(T1_a);
				if (!cut_b_only)
					T1->add_component(T1_c);
				// put T2 cut parts into T2
				if (!cut_b_only) {
					T2->add_component(T2_a);
				}
				// may have already been added
				if (cut_b) {
					T2->add_component(T2_b);
				}
				// problem if c is deleted
				if (add_T2_c) {
					T2->add_component(T2_c);
				}

				// may have already been added
				if (T2_b->is_leaf())
					singletons->push_back(T2_b);

			}
		}
	}
		// if the first component of the forests differ then we have to cut p
		if (T1->get_component(0)->get_twin() != T2->get_component(0)) {
			num_cut++;
			T1->add_rho();
			T2->add_rho();
		}
		return num_cut;
}

/* rSPR_worse_3_approx
 * Calculate an approximate maximum agreement forest and SPR distance
 * RETURN At most 3 times the rSPR distance
 * NOTE: destructive. The computed forests replace T1 and T2.
 */
int rSPR_worse_3_approx(Forest *T1, Forest *T2) {
	return rSPR_worse_3_approx(T1, T2, true);
}

int rSPR_worse_3_approx(Forest *T1, Forest *T2, bool sync) {
	// match up nodes of T1 and T2
	if (sync) {
		sync_twins(T1, T2);
	}
	// find sibling pairs of T1
	list<Node *> *sibling_pairs = T1->find_sibling_pairs();
	// find singletons of T2
	list<Node *> singletons = T2->find_singletons();
	list<pair<Forest,Forest> > AFs = list<pair<Forest,Forest> >();

	Forest *F1;
	Forest *F2;

	int ans = rSPR_worse_3_approx_hlpr(T1, T2, &singletons, sibling_pairs, &F1, &F2, true);

	F1->swap(T1);
	F2->swap(T2);
	sync_twins(T1,T2);


	delete sibling_pairs;
	delete F1;
	delete F2;
	return ans;
}

// rSPR_worse_3_approx recursive helper function
int rSPR_worse_3_approx_hlpr(Forest *T1, Forest *T2, list<Node *> *singletons, list<Node *> *sibling_pairs, Forest **F1, Forest **F2, bool save_forests) {
	#ifdef DEBUG_APPROX
		cout << "rSPR_worse_3_approx_hlpr" << endl;
					cout << "\tT1: ";
					T1->print_components_with_twins();
					cout << "\tT2: ";
					T2->print_components_with_twins();
					cout << "sibling pairs:";
					for (list<Node *>::iterator i = sibling_pairs->begin(); i != sibling_pairs->end(); i++) {
						cout << "  ";
						(*i)->print_subtree_hlpr();
					}
					cout << endl;
	#endif
	int num_cut = 0;
	UndoMachine um = UndoMachine();
	while(!singletons->empty() || !sibling_pairs->empty()) {
		// Case 1 - Remove singletons
		while(!singletons->empty()) {
			#ifdef DEBUG_APPROX
				cout << "Case 1" << endl;
			#endif

			Node *T2_a = singletons->back();
			singletons->pop_back();
			// find twin in T1
			Node *T1_a = T2_a->get_twin();
			// if this is in the first component of T_2 then
			// it is not really a singleton.
	// TODO: problem when we cluster and have a singleton as the
	//		first comp of T2
	//    NEED TO MODIFY CUTTING?
	// 		HERE AND IN BB?
			if (T2_a == T2->get_component(0))
				continue;

			Node *T1_a_parent = T1_a->parent();
			if (T1_a_parent == NULL)
				continue;
			bool potential_new_sibling_pair = T1_a_parent->is_sibling_pair();
			// cut the edge above T1_a
			um.add_event(new CutParent(T1_a));
			T1_a->cut_parent();
			um.add_event(new AddComponent(T1));
			T1->add_component(T1_a);
			//if (T1_a->get_sibling_pair_status() > 0)
			//	T1_a->clear_sibling_pair(sibling_pairs);
			//delete(T1_a);

			ContractEvent(&um, T1_a_parent);
			Node *node = T1_a_parent->contract();
			if (potential_new_sibling_pair && node->is_sibling_pair()){
				um.add_event(new AddToFrontSiblingPairs(sibling_pairs));
				sibling_pairs->push_front(node->rchild());
				sibling_pairs->push_front(node->lchild());
			}

	#ifdef DEBUG_APPROX
					cout << "\tT1: ";
					T1->print_components();
					cout << "\tT2: ";
					T2->print_components();
	#endif
		}
		if(!sibling_pairs->empty()) {
			Node *T1_a = sibling_pairs->back();
			sibling_pairs->pop_back();
			Node *T1_c = sibling_pairs->back();
			sibling_pairs->pop_back();
			um.add_event(new PopSiblingPair(T1_a, T1_c, sibling_pairs));

			//if (T1_a->get_sibling_pair_status() == 0 ||
			//		T1_c->get_sibling_pair_status() == 0) {
			//	continue;
			//}

			//T1_a->clear_sibling_pair_status();
			//T1_c->clear_sibling_pair_status();
			if (T1_a->parent() == NULL || T1_c->parent() == NULL || T1_a->parent() != T1_c->parent()) {
				continue;
			}
			Node *T1_ac = T1_a->parent();
			// lookup in T2 and determine the case
			Node *T2_a = T1_a->get_twin();
			Node *T2_c = T1_c->get_twin();

			#ifdef DEBUG_APPROX
				cout << "Fetching sibling pair" << endl;
				T1_ac->print_subtree();
				cout << "T2_a" << ": ";
				cout << " d=" << T2_a->get_depth() << " ";
				T2_a->print_subtree();
				cout << "T1_c" << ": ";
				T1_c->print_subtree();
				cout << "T2_c" << ": ";
				cout << " d=" << T2_c->get_depth() << " ";
				T2_c->print_subtree();
			#endif

			// Case 2 - Contract identical sibling pair
			if (T2_a->parent() != NULL && T2_a->parent() == T2_c->parent()) {
				#ifdef DEBUG_APPROX
					cout << "Case 2" << endl;
					T1->print_components();
					T2->print_components();
				#endif
				Node *T2_ac = T2_a->parent();
				um.add_event(new ContractSiblingPair(T1_ac));
				um.add_event(new ContractSiblingPair(T2_ac));
				T1_ac->contract_sibling_pair_undoable();
				T2_ac->contract_sibling_pair_undoable();
				um.add_event(new SetTwin(T1_ac));
				um.add_event(new SetTwin(T2_ac));
				T1_ac->set_twin(T2_ac);
				T2_ac->set_twin(T1_ac);
				//T1->add_deleted_node(T1_a);
				//T1->add_deleted_node(T1_c);
				//T2->add_deleted_node(T2_a);
				//T2->add_deleted_node(T2_c);

				// check if T2_ac is a singleton
				if (T2_ac->is_singleton() && !T1_ac->is_singleton() && T2_ac != T2->get_component(0))
					singletons->push_back(T2_ac);
				// check if T1_ac is part of a sibling pair
				if (T1_ac->parent() != NULL && T1_ac->parent()->is_sibling_pair()) {
					um.add_event(new AddToSiblingPairs(sibling_pairs));
					sibling_pairs->push_back(T1_ac->parent()->lchild());
					sibling_pairs->push_back(T1_ac->parent()->rchild());
				}
			}
			// Case 3
			else {
				#ifdef DEBUG_APPROX
					cout << "Case 3" << endl;
				#endif
				
				//  ensure T2_a is below T2_c
				if ((T2_a->get_depth() < T2_c->get_depth()
						&& T2_c->parent() != NULL)
						|| T2_a->parent() == NULL) {
					#ifdef DEBUG_APPROX
						cout << "swapping" << endl;
					#endif
				
					swap(&T1_a, &T1_c);
					swap(&T2_a, &T2_c);

				}
				else if (T2_a->get_depth() == T2_c->get_depth()) {
					if (T2_a->parent() && T2_c->parent() &&
							(T2_a->parent()->get_depth() <
							T2_c->parent()->get_depth())) {
					swap(&T1_a, &T1_c);
					swap(&T2_a, &T2_c);
					}
				}

				// get T2_b
				Node *T2_ab = T2_a->parent();
				Node *T2_b = T2_ab->rchild();
				if (T2_b == T2_a)
					T2_b = T2_ab->lchild();

				#ifdef DEBUG_APPROX
				cout << "T2_b" << ": ";
				cout.flush();
				T2_b->print_subtree();
			#endif
				// cut T1_a, T1_c, T2_a, T2_b, T2_c

				bool cut_b_only = false;
				if (T2_a->parent() != NULL && T2_a->parent()->parent() != NULL && T2_a->parent()->parent() == T2_c->parent()) {
					cut_b_only = true;
					um.add_event(new AddToSiblingPairs(sibling_pairs));
					sibling_pairs->push_back(T1_c);
					sibling_pairs->push_back(T1_a);
				}

				Node *node;

				if (!cut_b_only) {
					um.add_event(new CutParent(T1_a));
					T1_a->cut_parent();

					ContractEvent(&um, T1_ac);
					node = T1_ac->contract();

					um.add_event(new CutParent(T1_c));
					T1_c->cut_parent();


					ContractEvent(&um, node);
					node = node->contract();

					// contract parents
					// check for T1_ac sibling pair
					if (node && node->is_sibling_pair()){
						um.add_event(new AddToSiblingPairs(sibling_pairs));
						sibling_pairs->push_back(node->lchild());
						sibling_pairs->push_back(node->rchild());
					}
				}

				bool same_component = true;
				if (APPROX_CHECK_COMPONENT)
					same_component = (T2_a->find_root() == T2_c->find_root());

				Node *T2_ab_parent = T2_ab->parent();
				node = T2_ab;
				if (!cut_b_only) {
					um.add_event(new CutParent(T2_a));
					T2_a->cut_parent();

					//ContractEvent(&um, T2_ab);
					//node = T2_ab->contract();
				}
				bool cut_b = false;
				if (same_component && T2_ab_parent != NULL) {
					um.add_event(new CutParent(T2_b));
					T2_b->cut_parent();
					//ContractEvent(&um, node);
					//node = node->contract();
					cut_b = true;
				}
				// T2_b will move up after contraction
				else {
					T2_b = T2_b->parent();
				}
					ContractEvent(&um, node);
					node = node->contract();
				// check for T2 parents as singletons
				if (node != NULL && node->is_singleton()
						&& node != T2->get_component(0))
					singletons->push_back(node);

				// if T2_c is gone then its replacement is in singleton list
				// contract might delete old T2_c, see where it is
				bool add_T2_c = true;
				T2_c = T1_c->get_twin();
				// ignore T2_c if it is a singleton
				if (T2_c != node && T2_c->parent() != NULL && !cut_b_only) {

					Node *T2_c_parent = T2_c->parent();
					um.add_event(new CutParent(T2_c));
					T2_c->cut_parent();
					ContractEvent(&um, T2_c_parent);
					node = T2_c_parent->contract();
					if (node != NULL && node->is_singleton()
							&& node != T2->get_component(0))
						singletons->push_back(node);
				}
				else {
					add_T2_c = false;
				}

				
				if (!cut_b_only) {
					um.add_event(new AddComponent(T1));
					T1->add_component(T1_a);
					um.add_event(new AddComponent(T1));
					T1->add_component(T1_c);
					// put T2 cut parts into T2
					um.add_event(new AddComponent(T2));
					T2->add_component(T2_a);
					// may have already been added
				}
				if (cut_b) {
					um.add_event(new AddComponent(T2));
					T2->add_component(T2_b);
				}
				// problem if c is deleted
				if (add_T2_c) {
					um.add_event(new AddComponent(T2));
					T2->add_component(T2_c);
				}

				// may have already been added
				if (T2_b->is_leaf())
					singletons->push_back(T2_b);

				num_cut+=3;

			}
		}
	}
		// if the first component of the forests differ then we have cut p
		if (T1->get_component(0)->get_twin() != T2->get_component(0)) {
			if (!T1->contains_rho()) {
				um.add_event(new AddRho(T1));
				um.add_event(new AddRho(T2));
				T1->add_rho();
				T2->add_rho();
			}
			else
				// hack to ignore rho when it shouldn't be in a cluster
				num_cut -=3;
		}
		if (save_forests) {
			*F1 = new Forest(T1);
			*F2 = new Forest(T2);
		}
		 um.undo_all();
		 /*
		 while(um.num_events() > 0) {
				cout << "Undo step " << um.num_events() << endl;
				cout << "T1: ";
				T1->print_components();
				cout << "T2: ";
				T2->print_components();
					cout << "sibling pairs:";
					for (list<Node *>::iterator i = sibling_pairs->begin(); i != sibling_pairs->end(); i++) {
						cout << "  ";
						(*i)->print_subtree_hlpr();
					}
					cout << endl;
				cout << endl;
			 um.undo();
		 }
		 */
//		 for(int i = 0; i < T1->num_components(); i++)
//		 	T1->get_component(i)->fix_parents();
//		 for(int i = 0; i < T2->num_components(); i++)
//		 	T2->get_component(i)->fix_parents();
		return num_cut;
}

int rSPR_branch_and_bound(Forest *T1, Forest *T2) {
	return rSPR_branch_and_bound_range(T1, T2, MAX_SPR);
}


int rSPR_branch_and_bound_range(Forest *T1, Forest *T2, int end_k) {
	string problem_key;
	unordered_map<string,ProblemSolution>::iterator i;

	if (MEMOIZE) {
		problem_key = T1->str() + ":" + T2->str();
		i = memoized_clusters.find(problem_key);
		if (i != memoized_clusters.end()) {
			//cout << "already solved: " << endl;
			//cout << problem_key << endl;
			//cout << i->second.T2 << endl;
			//cout << "start" << endl;
			Forest *new_T1 = build_finished_forest(i->second.T1);
			//cout << "middle" << endl;
			Forest *new_T2 = build_finished_forest(i->second.T2);
			//cout << "end" << endl;
			T1->swap(new_T1);
			T2->swap(new_T2);
			sync_twins(T1, T2);
			delete new_T1;
			delete new_T2;
			return i->second.k;
		}
	}
	Forest F1 = Forest(T1);
	Forest F2 = Forest(T2);
	int approx_spr = rSPR_worse_3_approx(&F1, &F2);
	int min_spr = approx_spr / 3;
	int exact_spr = rSPR_branch_and_bound_range(T1, T2, min_spr, end_k);
	if (MEMOIZE && exact_spr >= 0 && i == memoized_clusters.end()) {
		//string solution_key = T1->str() + ":" + T2->str();
		memoized_clusters.insert(make_pair(problem_key,
				ProblemSolution(T1,T2,exact_spr)));
	}

	return exact_spr;
}
	
int rSPR_branch_and_bound_range(Forest *T1, Forest *T2, int start_k,
		int end_k) {
	int exact_spr = -1;
	bool in_main = MAIN_CALL;
	MAIN_CALL = false;
	int k;
	for(k = start_k; k <= end_k; k++) {
		if (in_main) {
			cout << " " << k;
			cout.flush();
		}
		Forest F1 = Forest(T1);
		Forest F2 = Forest(T2);
		exact_spr = rSPR_branch_and_bound(&F1, &F2, k);
		//if (exact_spr >= 0 || k == end_k) {
		if (exact_spr >= 0) {
			F1.swap(T1);
			F2.swap(T2);
			break;
		}
	}
	if (in_main)
		cout << endl;
	if (k > end_k)
		k = -1;
	return k;
}

/* rSPR_branch_and_bound
 * Calculate a maximum agreement forest and SPR distance
 * Uses a branch and bound optimization to not explore paths
 * guaranteed to be incorrect based on rspr_3_approx
 * RETURN The rSPR distance
 * NOTE: destructive. The computed forests replace T1 and T2.
 */
int rSPR_branch_and_bound(Forest *T1, Forest *T2, int k) {
	// find sibling pairs of T1
//	cout << "foo1" << endl;
	sync_twins(T1, T2);
//	cout << "foo2" << endl;
	list<Node *> *sibling_pairs = T1->find_sibling_pairs();
//	cout << "foo3" << endl;
	// find singletons of T2
	list<Node *> singletons = T2->find_singletons();
	list<pair<Forest,Forest> > AFs = list<pair<Forest,Forest> >();
//	cout << "foo4" << endl;
	int final_k = 
		rSPR_branch_and_bound_hlpr(T1, T2, k, sibling_pairs, &singletons, false, &AFs);
//		cout << "foo" << endl;
	// TODO: this is a cheap hack
	if (!AFs.empty()) {
		AFs.front().first.swap(T1);
		AFs.front().second.swap(T2);
		sync_twins(T1,T2);

		if (ALL_MAFS
#ifdef DEBUG
				|| true
#endif
				) {
			cout << endl << endl << "FOUND ANSWERS" << endl;
			// TODO: this is a cheap hack
			for (auto x = AFs.begin(); x != AFs.end(); x++) {
				cout << "\tT1: ";
				x->first.print_components();
				cout << "\tT2: ";
				x->second.print_components();
			}
		}
	}
	if (final_k >= 0)
		final_k = k - final_k;
	delete sibling_pairs;
	return final_k;
}


// rSPR_branch_and_bound recursive helper function
int rSPR_branch_and_bound_hlpr(Forest *T1, Forest *T2, int k,
		list<Node *> *sibling_pairs, list<Node *> *singletons,
		bool cut_b_only, list<pair<Forest,Forest> > *AFs) {
	#ifdef DEBUG
	cout << "rSPR_branch_and_bound_hlpr()" << endl;
	cout << "\tT1: ";
	T1->print_components();
	cout << "\tT2: ";
	T2->print_components();
	cout << "K=" << k << endl;
	cout << "sibling pairs:";
	for (list<Node *>::iterator i = sibling_pairs->begin(); i != sibling_pairs->end(); i++) {
		cout << "  ";
		(*i)->print_subtree_hlpr();
	}
	cout << endl;
	#endif

	UndoMachine um = UndoMachine();

	
	while(!singletons->empty() || !sibling_pairs->empty()) {
		// Case 1 - Remove singletons
		while(!singletons->empty()) {
			Node *T2_a = singletons->back();
			#ifdef DEBUG
				cout << "Case 1" << endl;
				cout << "a " << T2_a->str_subtree() << endl;
			#endif

			singletons->pop_back();
			// find twin in T1
			Node *T1_a = T2_a->get_twin();

			//if (T1_a->get_sibling_pair_status() > 0)
			//	cout << T1_a->get_sibling(sibling_pairs) << endl;
			// if this is in the first component of T_2 then
			// it is not really a singleton.
			Node *T1_a_parent = T1_a->parent();
			if (T1_a_parent == NULL)
				continue;
			bool potential_new_sibling_pair = T1_a_parent->is_sibling_pair();
			if (T2_a == T2->get_component(0)) {
				// TODO: should we do this when it happens?
				if (!T1->contains_rho()) {
					um.add_event(new AddRho(T1));
					um.add_event(new AddRho(T2));
					T1->add_rho();
					T2->add_rho();
					k--;
					#ifdef DEBUG
					cout << "adding p element, k=" << k << endl;
					#endif
				}
			}

			// cut the edge above T1_a
			um.add_event(new CutParent(T1_a));
			T1_a->cut_parent();

			um.add_event(new AddComponent(T1));
			T1->add_component(T1_a);
			//if (T1_a->get_sibling_pair_status() > 0) {
			//	um.add_event(new ClearSiblingPair(T1_a, T1_a->get_sibling(sibling_pairs)));
			//	T1_a->clear_sibling_pair(sibling_pairs);
			//}
			ContractEvent(&um, T1_a_parent);
			Node *node = T1_a_parent->contract();
			if (potential_new_sibling_pair && node->is_sibling_pair()){
				um.add_event(new AddToFrontSiblingPairs(sibling_pairs));
				sibling_pairs->push_front(node->rchild());
				sibling_pairs->push_front(node->lchild());
			}
			#ifdef DEBUG
				cout << "\tT1: ";
				T1->print_components();
				cout << "\tT2: ";
				T2->print_components();
			#endif
		}
		if(!sibling_pairs->empty()) {
			Node *T1_a = sibling_pairs->back();
			sibling_pairs->pop_back();
			Node *T1_c = sibling_pairs->back();
			sibling_pairs->pop_back();
			
			//if (T1_a->get_sibling_pair_status() == 0 ||
			//		T1_c->get_sibling_pair_status() == 0) {
			//	um.add_event(new PopClearedSiblingPair(T1_a, T1_c, sibling_pairs));
			//	continue;
			//}
			um.add_event(new PopSiblingPair(T1_a, T1_c, sibling_pairs));
			//T1_a->clear_sibling_pair_status();
			//T1_c->clear_sibling_pair_status();

			//if (T1->get_component(0)->str() != "") {
			//	cout << "FOO!!!" << endl;
			//	sibling_pairs->clear();
			//}
//			if (T1_a->parent() != NULL)
//				cout << "a_p: " << T1_a->parent()->str_subtree() << endl;
//			if (T1_c->parent() != NULL)
//				cout << "c_p: " << T1_c->parent()->str_subtree() << endl;
			if (T1_a->parent() == NULL || T1_a->parent() != T1_c->parent()) {
				continue;
			}
			Node *T1_ac = T1_a->parent();
			// lookup in T2 and determine the case
			Node *T2_a = T1_a->get_twin();
			Node *T2_c = T1_c->get_twin();

			if (T2_a->parent() != NULL && T2_a->parent() == T2_c->parent()) {
				#ifdef DEBUG
					cout << "Case 2" << endl;
					T1_ac->print_subtree();
				#endif
				Node *T2_ac = T2_a->parent();

				um.add_event(new ContractSiblingPair(T1_ac));
				um.add_event(new ContractSiblingPair(T2_ac));
				T1_ac->contract_sibling_pair_undoable();
				T2_ac->contract_sibling_pair_undoable();

				um.add_event(new SetTwin(T1_ac));
				um.add_event(new SetTwin(T2_ac));
				T1_ac->set_twin(T2_ac);
				T2_ac->set_twin(T1_ac);
				//T1->add_deleted_node(T1_a);
				//T1->add_deleted_node(T1_c);
				//T2->add_deleted_node(T2_a);
				//T2->add_deleted_node(T2_c);

				// check if T2_ac is a singleton
				if (T2_ac->is_singleton() && !T1_ac->is_singleton() && T2_ac != T2->get_component(0))
					singletons->push_back(T2_ac);
				// check if T1_ac is part of a sibling pair
				if (T1_ac->parent() != NULL && T1_ac->parent()->is_sibling_pair()) {
					um.add_event(new AddToSiblingPairs(sibling_pairs));
					sibling_pairs->push_back(T1_ac->parent()->lchild());
					sibling_pairs->push_back(T1_ac->parent()->rchild());
				}
				#ifdef DEBUG
					cout << "\tT1: ";
					T1->print_components();
					cout << "\tT2: ";
					T2->print_components();
				#endif
			}
			/* need to copy trees and lists for branching
			 * use forest copy constructor for T1 and T2 giving T1' and T2'
			 * T1' twins are in T2, and same for T2' and T1.
			 * singleton list will be empty except for maybe above the cut,
			 * so this can be created.
			 * fix one set of twins (T2->T1' or T1->T2' not sure)
			 * exploit chained twin relationship to copy sibling pair list
			 * fix other set of twins
			 * swap T2 and T2' root nodes
			 * now do the cut
			 *
			 * note: don't copy for 3rd cut, is a waste
			 */

			// Case 3
			// note: guaranteed that singleton list is empty
			else {
				if (k <= 0) {
					if ((T2_c->parent() != NULL && T2_a->parent() != NULL)|| !T2->contains_rho()) {
						singletons->clear();
						um.undo_all();
						return k-1;
					}
				}
				Forest *best_T1;
				Forest *best_T2;
				int best_k = -1;
				int answer_a = -1;
				int answer_b = -1;
				int answer_c = -1;
				bool cut_ab_only = false;
				int undo_state = um.num_events();
				//  ensure T2_a is below T2_c
				if ((T2_a->get_depth() < T2_c->get_depth()
						&& T2_c->parent() != NULL)
						|| T2_a->parent() == NULL) {
					swap(&T1_a, &T1_c);
					swap(&T2_a, &T2_c);
				}
				else if (T2_a->get_depth() == T2_c->get_depth()) {
					if (T2_a->parent() && T2_c->parent() &&
							(T2_a->parent()->get_depth() <
							T2_c->parent()->get_depth()
							//|| (T2_c->parent()->parent()
							//&& T2_c->parent()->parent() == T2_a->parent())
							)) {
					swap(&T1_a, &T1_c);
					swap(&T2_a, &T2_c);
					}
				}
				Node *T2_b = T2_a->parent()->rchild();
				if (T2_b == T2_a)
					T2_b = T2_a->parent()->lchild();

			if (CUT_ONE_B) {
				if (T2_a->parent()->parent() == T2_c->parent()
					&& T2_c->parent() != NULL)
					cut_b_only=true;
			}
			if (CUT_ONE_AB) {
				if (T2_a->parent()->parent() == T2_c->parent()
					&& T2_c->parent() != NULL)
					cut_ab_only=true;
			}
				#ifdef DEBUG
					cout << "Case 3" << endl;
					cout << "\tT1: ";
					T1->print_components();
					cout << "\tT2: ";
					T2->print_components();
					cout << "K=" << k << endl;
					cout << "sibling pairs:";
					for (list<Node *>::iterator i = sibling_pairs->begin(); i != sibling_pairs->end(); i++) {
						cout << "  ";
						(*i)->print_subtree_hlpr();
					}
					cout << endl;
					cout << "\tcut_b_only=" << cut_b_only << endl;
					cout << "\tT2_a " << T2_a->str() << " "
						<< T2_a->get_depth() << endl;
					cout << "\tT2_c " << T2_c->str() << " "
						<< T2_c->get_depth() << endl;
					cout << "\tT2_b " << T2_b->str_subtree() << " "
						<< T2_b->get_depth() << endl;
				#endif
			


				// copy elements
					/*
				Forest *T1_copy;
				Forest *T2_copy;
				list<Node *> *sibling_pairs_copy;
				Node *T1_a_copy;
				Node *T1_c_copy;
				Node *T2_a_copy;
				Node *T2_c_copy;
				*/
				//list<Node *> *singletons_copy = new list<Node *>();

				// make copies for the approx
				// be careful we do not kill real T1 and T2
				// ie use the copies
				if (BB) {
				um.add_event(new AddToSiblingPairs(sibling_pairs));
				sibling_pairs->push_back(T1_c);
				sibling_pairs->push_back(T1_a);
				//copy_trees(&T1, &T2, &sibling_pairs, &T1_a, &T1_c, &T2_a, &T2_c,
				//		&T1_copy, &T2_copy, &sibling_pairs_copy,
				//		&T1_a_copy, &T1_c_copy, &T2_a_copy, &T2_c_copy);
				//Forest T1_copy = Forest(T1);
				//Forest T2_copy = Forest(T2);
				//list<Node *> *sibling_pairs_copy = T1->find_sibling_pairs();
				//sync_twins(&T1_copy, &T2_copy);
				//int approx_spr = rSPR_worse_3_approx(&T1_copy, &T2_copy);
				/*
				cout << "FOO" << endl;
				cout << "T1: ";
				T1->print_components();
				cout << "T2: ";
				T2->print_components();
					cout << "sibling pairs:";
					for (list<Node *>::iterator i = sibling_pairs->begin(); i != sibling_pairs->end(); i++) {
						cout << "  ";
						(*i)->print_subtree_hlpr();
					}
					cout << endl;
					*/
				int approx_spr = rSPR_worse_3_approx_hlpr(T1, T2,
						singletons, sibling_pairs, NULL, NULL, false);
				/*
				cout << "T1: ";
				T1->print_components();
				cout << "T2: ";
				T2->print_components();
					cout << "sibling pairs:";
					for (list<Node *>::iterator i = sibling_pairs->begin(); i != sibling_pairs->end(); i++) {
						cout << "  ";
						(*i)->print_subtree_hlpr();
					}
					cout << endl;
				cout << endl;
				*/
				#ifdef DEBUG
					cout << "\tT1: ";
					T1->print_components();
					cout << "\tT2: ";
					T2->print_components();
					cout << "approx =" << approx_spr << endl;
				#endif
					if (approx_spr  >  3*k){
						#ifdef DEBUG
							cout << "approx failed" << endl;
						#endif
						um.undo_all();
						return -1;
					}
				um.undo_to(undo_state);
				}
				
			if (CLUSTER_REDUCTION && (MAX_CLUSTERS < 0 || NUM_CLUSTERS < MAX_CLUSTERS)) {
				// clean up singletons
				// TODO: this is duplication
				/*
			while(!singletons->empty()) {
				Node *T2_a = singletons->back();
				singletons->pop_back();
				// find twin in T1
				Node *T1_a = T2_a->get_twin();
				// if this is in the first component of T_2 then
				// it is not really a singleton.
				Node *T1_a_parent = T1_a->parent();
				if (T1_a_parent == NULL)
					continue;
				bool potential_new_sibling_pair = T1_a_parent->is_sibling_pair();
				if (T2_a == T2->get_component(0)) {
					T1->add_rho();
					T2->add_rho();
					k--;
				}
	
				// cut the edge above T1_a
				T1_a->cut_parent();
				T1->add_component(T1_a);
				Node *node = T1_a_parent->contract();
				if (potential_new_sibling_pair && node->is_sibling_pair()){
					sibling_pairs->push_front(node->lchild());
					sibling_pairs->push_front(node->rchild());
				}
			}
			*/
	//			cout << "foo" << endl;
	//			cout << "foo2" << endl;
//				cout << "\tT1: ";
//				T1->print_components();
//				cout << "\tT2: ";
//				T2->print_components();
				sync_interior_twins_real(T1, T2);
				list<Node *> *cluster_points = find_cluster_points(T1);
				//cluster_points->erase(++cluster_points->begin(),cluster_points->end());
	
				// TODO: could this be faster by using the approx to allocate
				// a certain amount of the k to different clusters?
				// TODO: write pseudocode for what we need
				// TODO: then implement it
				// NOTE: need to make a list of ClusterInstances and then
				// solve each.
				// TODO: where should we do this? Just before we would
				// normally branch?
	
//				cout << "k=" << k << endl;
//				cout << "cp=" << cluster_points->size() << endl;
				if (!cluster_points->empty()) {
					NUM_CLUSTERS++;
					sibling_pairs->clear();
#ifdef DEBUG_CLUSTERS
					cout << "CLUSTERS" << endl;
					for(int j = 0; j < 70; j++) {
						cout << "*";
					}
					cout << endl;
					for(list<Node *>::iterator i = cluster_points->begin();
							i != cluster_points->end(); i++) {
						cout << (*i)->str_subtree() << endl;
						cout << (*i)->get_twin()->str_subtree() << endl;
						for(int j = 0; j < 70; j++) {
							cout << "*";
						}
						cout << endl;
					}
					cout << endl;
#endif
	
					list<ClusterInstance> clusters =
						cluster_reduction(T1, T2, cluster_points);
	
					// TODO: make it so we don't need this?
					T1->unsync_interior();
					T2->unsync_interior();
					while(!clusters.empty()) {
						ClusterInstance cluster = clusters.front();
						clusters.pop_front();
						cluster.F1->unsync_interior();
						cluster.F2->unsync_interior();
#ifdef DEBUG_CLUSTERS
						cout << "CLUSTER_START" << endl;
						cout << &(*cluster.F1->get_component(0)) << endl;
						if (!clusters.empty())
							cout << &(*clusters.front().F1->get_component(0)) << endl;
						cout << "\tF1: ";
						cluster.F1->print_components();
						cout << "\tF2: ";
						cluster.F2->print_components();
						cout << "K=" << k << endl;
#endif
						int cluster_spr = -1;
						if (k >= 0) {
							// hack for clusters with no rho
//							cout << __LINE__ << endl;
//							cout << cluster.F2_cluster_node << endl;
//							cout << cluster.F2_has_component_zero << endl;
							if ((cluster.F2_cluster_node == NULL
										|| (cluster.F2_cluster_node->is_leaf()
												&& cluster.F2_cluster_node->parent() == NULL
												&& cluster.F2_cluster_node->
												get_num_clustered_children() <= 1
												&& (cluster.F2_cluster_node !=
														cluster.F2_cluster_node->get_forest()->
															get_component(0))))
										&& cluster.F2_has_component_zero == false) {
//							cout << __LINE__ << endl;
								cluster.F1->add_rho();
								cluster.F2->add_rho();
							}
							cluster_spr = rSPR_branch_and_bound_range(cluster.F1,
									cluster.F2, k);
							if (cluster_spr >= 0) {
//							cout << "cluster k=" << cluster_spr << endl;
//							cout << "\tF1: ";
//							cluster.F1->print_components();
//							cout << "\tF2: ";
//							cluster.F2->print_components();
								k -= cluster_spr;
							}
							else {
								k = -1;
							}
						}
						if (k > -1) {
//							cout << "\tF1: ";
//							T1->print_components();
//							cout << "\tF2: ";
//							T2->print_components();
								if (!cluster.is_original()) {
									int adjustment = cluster.join_cluster(T1, T2);
									k -= adjustment;
									delete cluster.F1;
									delete cluster.F2;
	
	//						cout << cluster.F1_cluster_node->str_subtree() << endl;
	//						cout << cluster.F2_cluster_node->str_subtree() << endl;
	//						Node *p = cluster.F1_cluster_node;
	//						while (p->parent() != NULL)
	//							p = p->parent();
	//						cout << &(*p) << endl;
	//						cout << p->str_subtree() << endl;
	
	
							//cout << "\tF1: ";
							//T1->print_components();
							//cout << "\tF2: ";
							//T2->print_components();
								}
//						else {
//							cout << "original" << endl;
//						}
							}
							else {
								if (!cluster.is_original()) {
									//if (cluster.F1_cluster_node != NULL)
									//	cluster.F1_cluster_node->contract();
									//if (cluster.F2_cluster_node != NULL)
									//	cluster.F2_cluster_node->contract();
									delete cluster.F1;
									delete cluster.F2;
								}
							}
					}
					delete cluster_points;
//					cout << "returning k=" << k << endl;
					NUM_CLUSTERS--;
					return k;
				}
				else {
					T1->unsync_interior();
					T2->unsync_interior();
					NUM_CLUSTERS--;
				}
				delete cluster_points;
	
	//			cout << "done" << endl;
			}
				 // make copies for the branching
			/*
				copy_trees(&T1, &T2, &sibling_pairs, &T1_a, &T1_c, &T2_a, &T2_c,
						&T1_copy, &T2_copy, &sibling_pairs_copy,
					&T1_a_copy, &T1_c_copy, &T2_a_copy, &T2_c_copy);
					*/

				// cut T2_a
				Node *T2_ab = T2_a->parent();
				Node *node;
				if (cut_b_only == false) {
					um.add_event(new CutParent(T2_a));
					T2_a->cut_parent();
					ContractEvent(&um, T2_ab);
					node = T2_ab->contract();
					if (node != NULL && node->is_singleton() &&
							node != T2->get_component(0))
						singletons->push_back(node);
					um.add_event(new AddComponent(T2));
					T2->add_component(T2_a);
					singletons->push_back(T2_a);
					answer_a =
						rSPR_branch_and_bound_hlpr(T1, T2, k-1,
								sibling_pairs, singletons, false, AFs);
				}
				best_k = answer_a;
				best_T1 = T1;
				best_T2 = T2;

				um.undo_to(undo_state);

				//load the copy
				/*
				T1 = T1_copy;
				T2 = T2_copy;
				T1_a = T1_a_copy;
				T1_c = T1_c_copy;
				T2_a = T2_a_copy;
				T2_c = T2_c_copy;
				sibling_pairs = sibling_pairs_copy;
				singletons = new list<Node *>();
				*/


				// make copies for the branching
				/*
				copy_trees(&T1, &T2, &sibling_pairs, &T1_a, &T1_c, &T2_a, &T2_c,
						&T1_copy, &T2_copy, &sibling_pairs_copy,
						&T1_a_copy, &T1_c_copy, &T2_a_copy, &T2_c_copy);
						*/

				// get T2_b
				T2_ab = T2_a->parent();
				T2_b = T2_ab->rchild();

				if (T2_b == T2_a)
					T2_b = T2_ab->lchild();

				if (!CUT_AC_SEPARATE_COMPONENTS || T2_a->find_root() == T2_c->find_root()) {
					// cut T2_b
					um.add_event(new CutParent(T2_b));
					T2_b->cut_parent();
					ContractEvent(&um, T2_ab);
					node = T2_ab->contract();
					if (node != NULL && node->is_singleton()
							&& node != T2->get_component(0))
						singletons->push_back(node);
					T2->add_component(T2_b);
					um.add_event(new AddComponent(T2));
					if (T2_b->is_leaf())
						singletons->push_back(T2_b);
					um.add_event(new AddToSiblingPairs(sibling_pairs));
					sibling_pairs->push_back(T1_a);
					sibling_pairs->push_back(T1_c);
					if (CUT_ALL_B) {
						answer_b =
							rSPR_branch_and_bound_hlpr(T1, T2, k-1,
									sibling_pairs, singletons, true, AFs);
					}
					else {
						answer_b =
							rSPR_branch_and_bound_hlpr(T1, T2, k-1,
									sibling_pairs, singletons, false, AFs);
					}
				}
				if (answer_b > best_k
						|| (answer_b == best_k
							&& PREFER_RHO
							&& T2->contains_rho() )) {
					best_k = answer_b;
					//swap(&best_T1, &T1);
					//swap(&best_T2, &T2);
				}

				um.undo_to(undo_state);

				/*
				delete T1;
				delete T2;
				delete sibling_pairs;
				delete singletons;
				*/


				// load the copy
				/*
				T1 = T1_copy;
				T2 = T2_copy;
				T1_a = T1_a_copy;
				T1_c = T1_c_copy;
				T2_a = T2_a_copy;
				T2_c = T2_c_copy;
				sibling_pairs = sibling_pairs_copy;
				singletons = new list<Node *>();
				*/

					if (T2_c->parent() != NULL) {
						Node *T2_c_parent = T2_c->parent();
						um.add_event(new CutParent(T2_c));
						T2_c->cut_parent();
						ContractEvent(&um, T2_c_parent);
						node = T2_c_parent->contract();
						if (node != NULL && node->is_singleton()
								&& node != T2->get_component(0))
							singletons->push_back(node);
						um.add_event(new AddComponent(T2));
						T2->add_component(T2_c);
					}
					else {
						// don't decrease k
						k++;
					}
				if (cut_b_only == false && cut_ab_only == false) {
					singletons->push_back(T2_c);
					answer_c =
						rSPR_branch_and_bound_hlpr(T1, T2, k-1,
								sibling_pairs, singletons, false, AFs);
				if (answer_c > best_k
						|| (answer_c == best_k
							&& PREFER_RHO
							&& T2->contains_rho() )) {
					best_k = answer_c;
					//swap(&best_T1, &T1);
					//swap(&best_T2, &T2);
				}
				}
				/*
				delete T1;
				delete T2;
				delete sibling_pairs;
				delete singletons;
				*/

				um.undo_to(undo_state);

				//T1 = best_T1;
				//T2 = best_T2;

				um.undo_all();
				singletons->clear();
				return best_k;
			}
			cut_b_only = false;
		}
	}

	if (k >= 0) {
		if (PREFER_RHO && !AFs->empty() && !AFs->front().first.contains_rho() && T1->contains_rho()) {
			if (!ALL_MAFS)
				AFs->clear();
			AFs->push_front(make_pair(Forest(T1),Forest(T2)));
		}
		else if (ALL_MAFS || AFs->empty()) {
			AFs->push_back(make_pair(Forest(T1),Forest(T2)));
		}
	}

	um.undo_all();

	return k;
}