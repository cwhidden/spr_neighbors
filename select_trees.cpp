#include <iostream>
#include <fstream>
#include <list>

using namespace std;

//#define DEBUG 0
bool OUTPUT_EDGES = false;

int main(int argc, char **argv) {
	// tree_number_file as args
	if (argc < 2) {
		cout << "usage: select_trees.cpp <tree_number_file> < trees" << endl;
		exit(0);
	}
	string tree_number_filename = argv[1];

	fstream tree_number_file;
	tree_number_file.open(tree_number_filename, fstream::in);

	list<int> trees = list<int>();

	string line;
	while(getline(tree_number_file, line)) {
		int num = atoi(line.c_str());
		trees.push_back(num);
	}
	
	int current = 0;
	while (getline(cin, line)) {
		if (trees.front() == current) {
			trees.pop_front();
			cout << line << endl;
		}
		current++;
	}
}
