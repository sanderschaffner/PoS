#include <iostream>
#include <vector>
#include <algorithm>

#include "matrix.h"
#include "gurobi_c++.h"

using namespace std;

typedef double var;

/*
 * get edge-number depending on vertex-numbers
 */
int edge(int i, int j){
	if (i>j){
		int tmp = j;
		j = i;
		i = tmp;
	}

	if (i==0 && j==1)
		return 0;
	else if (i==1 && j==2)
		return 1;
	else if (i==2 && j==4)
		return 2;
	else if (i==3 && j==4)
		return 3;
	else if (i==3 && j==5)
		return 4;
	else if (i==0 && j==3)
		return 5;
	else if (i==1 && j==4)
		return 6;
	else if (i==2 && j==5)
		return 7;
}

int main(int argc, char *argv[]) {
	try {
		GRBEnv env = GRBEnv();

		GRBModel model = GRBModel(env);

		// Matrix of graph we want to get PoS
		Matrix<bool> graph(6,6);
		graph(0,1) = 1; graph(1,2) = 1; graph(2,3) = 1; graph(3,4) = 1; graph(4,5) = 1;
		graph(0,3) = 1; graph(1,4) = 1; graph(2,5) = 1;
		//cout << "matrix: " << endl << graph << endl;

		// Get all possible paths for each player in the graph


	} catch(GRBException e) {
		cout << "Error code = " << e.getErrorCode() << endl;
		cout << e.getMessage() << endl;
	} catch(...) {
		cout << "Exception during optimization" << endl;
	}

	return 0;
}