#include <iostream>
#include <fstream>
#include <map>
#include <boost/tokenizer.hpp>

using namespace std;

//#define DEBUG 0
bool OUTPUT_EDGES = false;

int main(int argc, char **argv) {
	// tree_number_file as args
	if (argc < 2) {
		cout << "usage: select_trees.cpp <tree_number_file> < edges" << endl;
		exit(0);
	}
	string tree_number_filename = argv[1];

	fstream tree_number_file;
	tree_number_file.open(tree_number_filename, fstream::in);

	string line;
	// construct old -> new number mapping
	int new_num = 0;
	map<int, int> num_map = map<int,int>();
	while(getline(tree_number_file, line)) {
		int old_num = atoi(line.c_str());
		num_map.insert(make_pair(old_num, new_num));
		new_num++;
	}
	
	// renumber edges and only keep valid ones
	int current = 0;
	boost::char_separator<char> sep(", ");
	while (getline(cin, line)) {
		boost::tokenizer<boost::char_separator<char> > tokens(line, sep);
		boost::tokenizer<boost::char_separator<char> >::iterator t = tokens.begin();
		if (t == tokens.end()) {
			continue;
		}
		int old_source = atoi((*t).c_str());
		t++;
		if (t == tokens.end()) {
			continue;
		}
		int old_target = atoi((*t).c_str());

		map<int,int>::iterator new_source = num_map.find(old_source);
		map<int,int>::iterator new_target = num_map.find(old_target);
		if (new_source != num_map.end() && new_target != num_map.end()) {
			cout << new_source->second << "," << new_target->second << endl;
		}
	}
}
