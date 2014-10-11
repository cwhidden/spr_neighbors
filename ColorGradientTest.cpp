#include <cstdio>
#include <iostream>
#include "ColorGradient.h"

using namespace std;

int main(int argc, char**argv) {
	int num_colors = atoi(argv[1]);

	ColorGradient Bu = ColorGradient();
	Bu.clearGradient();
	Bu.createBu();



	for(int i = 1; i <= num_colors; i++) {
		float value = (float)i / num_colors;
		cout << value;
		cout << ", ";
		cout << Bu.getHexColorAtValue(value);
		cout << endl;
	}
	cout << endl;
}
