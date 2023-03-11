#include <stdio.h>
#include <vector>
#include <algorithm>
#include <string>
#include <iostream>
using namespace std;

struct hi {
	int key;
	int value;
};
struct node {
	vector<hi*> ab;
};




int main() {
	
	FILE* fw;
	fw = fopen("testoutput.txt", "w");
	char level[20] = "level 0";
	fprintf(fw, "%s\n", level);
	int keyToDisplay;
	int abc[5] = { 1,2,3,4,5 };
	for (int i{ 0 }; i < 4 - 1; i++) {
		keyToDisplay = abc[i];
		fprintf(fw, "%d,", keyToDisplay);
	}

	return 0;
}