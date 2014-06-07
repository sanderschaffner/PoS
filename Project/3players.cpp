#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator> // for ostream_iterator

#include "gurobi_c++.h"
#include "matrix.h"
#include "paths.h"

using namespace std;

typedef double var;

/*
 * get edge-number depending on vertex-numbers
 * short, direct ones: 0: 0<->1, 1: 1<->2, 2: 2<->3, 3: 3<->4, 4: 4<->5
 * direct from source to target: 5: 0<->3, 6: 1<->4, 7: 2<->5
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
	else if (i==4 && j==5)
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

		///////////////////////////////////////////////////////////
		// Create Graph and find all possible paths for each player
		///////////////////////////////////////////////////////////

		// 1:
		// Matrix of graph we want to get PoS
		Matrix<bool> graph(6,6);
		graph(0,1) = 1; graph(1,2) = 1; graph(2,3) = 1; graph(3,4) = 1; graph(4,5) = 1;
		graph(0,3) = 1; graph(1,4) = 1; graph(2,5) = 1;
		// symmetric:
		graph(1,0) = 1; graph(2,1) = 1; graph(3,2) = 1; graph(4,3) = 1; graph(5,4) = 1;
		graph(3,0) = 1; graph(4,1) = 1; graph(5,2) = 1;
		//cout << "matrix: " << endl << graph << endl;

		// 2:
		// Get all possible paths for each player in the graph
		Paths paths_p1(0,3,6); // source, target, number of nodes in graph
		//paths_p1.print();
		Paths paths_p2(1,4,6); // source, target, number of nodes in graph
		Paths paths_p3(2,5,6); // source, target, number of nodes in graph
		paths_p1.getAllPaths(graph);
		paths_p2.getAllPaths(graph);
		paths_p3.getAllPaths(graph);

		//////////////////////////////////////////
		// Define optimization problem with GUROBI
		//////////////////////////////////////////

		GRBEnv env = GRBEnv();

		GRBModel model = GRBModel(env);

		// Define variables
		const double lowerBound = 0.001;
		const double upperBound = GRB_INFINITY;
		const int n_variables = 8;

		// Create variables
		// var_number: note x -> node y
		// short, direct ones: 0: 0<->1, 1: 1<->2, 2: 2<->3, 3: 3<->4, 4: 4<->5
		// direct from source to target: 5: 0<->3, 6: 1<->4, 7: 2<->5
		double lb[n_variables];
		fill_n(lb, n_variables, lowerBound);
		double ub[n_variables];
		fill_n(ub, n_variables, upperBound);
		GRBVar* v_edges = model.addVars(lb, ub, NULL, NULL, NULL, n_variables);
		model.update();

		// 3:
		// Inequalities saying that the minimum spanning tree is the line (optimum=={0to1to2to3to4to5})
		// So direct source to target paths have to be bigger than short ones
		model.addConstr(v_edges[edge(0,1)] <= v_edges[edge(0,3)]);
		model.addConstr(v_edges[edge(1,2)] <= v_edges[edge(0,3)]);
		model.addConstr(v_edges[edge(2,3)] <= v_edges[edge(0,3)]);
		model.addConstr(v_edges[edge(1,2)] <= v_edges[edge(1,4)]);
		model.addConstr(v_edges[edge(2,3)] <= v_edges[edge(1,4)]);
		model.addConstr(v_edges[edge(3,4)] <= v_edges[edge(1,4)]);
		model.addConstr(v_edges[edge(2,3)] <= v_edges[edge(2,5)]);
		model.addConstr(v_edges[edge(3,4)] <= v_edges[edge(2,5)]);
		model.addConstr(v_edges[edge(4,5)] <= v_edges[edge(2,5)]);
		// Cost of the spanning tree is 1 (optimum has cost one -> result of lp gives directly PoS)
		GRBLinExpr cost_p1_opt = v_edges[edge(0,1)] + v_edges[edge(1,2)]/2 + v_edges[edge(2,3)]/3;
		GRBLinExpr cost_p2_opt = v_edges[edge(1,2)]/2 + v_edges[edge(2,3)]/3 + v_edges[edge(3,4)]/2;
		GRBLinExpr cost_p3_opt = v_edges[edge(2,3)]/3 + v_edges[edge(3,4)]/2 + v_edges[edge(4,5)];
		model.addConstr(cost_p1_opt + cost_p2_opt + cost_p3_opt == 1);

		// 4:
		// Inequalities checking that the edges (0,3) (1,4) and (2,5) give a Nash equilibrium

	} catch(GRBException e) {
		cout << "Error code = " << e.getErrorCode() << endl;
		cout << e.getMessage() << endl;
	} catch(...) {
		cout << "Exception during optimization" << endl;
	}

	return 0;
}