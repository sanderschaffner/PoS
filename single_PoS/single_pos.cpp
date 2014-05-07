#include "gurobi_c++.h"
using namespace std;

int main(int argc, char *argv[]) {
	try {
		GRBEnv env = GRBEnv();

		GRBModel model = GRBModel(env);

		// Define variables
		const double lowerBound = 0.001;
		const double upperBound = GRB_INFINITY;

		// Create variables
		double lb[] = {lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound, lowerBound};
		double ub[] = {upperBound, upperBound, upperBound, upperBound, upperBound, upperBound, upperBound, upperBound};
		GRBVar* vars = model.addVars(lb, ub, NULL, NULL, NULL, 8);

		// Integrate new variables

		model.update();

		///////////////////////////
		// Set constraints
		// Opt: vars[0]-vars[5]
		// Nash: vars[3]-vars[7]
		///////////////////////////

		// Three constraints:
		// 1: Nash has to be Nash (Alternatives have to be bigger than assumed Nash-solution)
		// 2: Cost of optimal path has to be 1
		// 3: Cost(opt) <= Cost(Nash)

		/////
		// 1.
		/////

		// p1, p2 & p3 for Nash:
		GRBLinExpr cost_p1 = vars[6] + vars[7]/2 + vars[3]/3 + vars[4]/3;
		GRBLinExpr cost_p2 = vars[7]/2 + vars[3]/3 + vars[4]/3 + vars[5]/2;
		GRBLinExpr cost_p3 = vars[3]/3 + vars[4]/3 + vars[5]/2;

		// alternative p1
		GRBLinExpr cost_p1_1 = vars[0] + vars[2] + vars[4]/3;
		GRBLinExpr cost_p1_2 = vars[0] + vars[1] + vars[7]/2 + vars[3]/3 + vars[4]/3;
		GRBLinExpr cost_p1_3 = vars[6] + vars[1] + vars[2] + vars[4]/3;

		// alternative p2
		GRBLinExpr cost_p2_1 = vars[1] + vars[2] + vars[4]/3 + vars[5]/2;
		GRBLinExpr cost_p2_2 = vars[6]/2 + vars[0] + vars[2] + vars[4]/3 + vars[5]/2;

		// alternative p3
		GRBLinExpr cost_p3_1 = vars[7]/3 + vars[1] + vars[2] + vars[4]/3 + vars[5]/2;
		GRBLinExpr cost_p3_2 = vars[7]/3 + vars[6]/2 + vars[0] + vars[2] + vars[4]/3 + vars[5]/2;

		// Add constraints for Nash and each player
		model.addConstr(cost_p1 <= cost_p1_1);
		model.addConstr(cost_p1 <= cost_p1_2);
		model.addConstr(cost_p1 <= cost_p1_3);
		model.addConstr(cost_p2 <= cost_p2_1);
		model.addConstr(cost_p2 <= cost_p2_2);
		model.addConstr(cost_p3 <= cost_p3_1);
		model.addConstr(cost_p3 <= cost_p3_2);

		/////
		// 2.
		/////

		// Add constraint that cost(opt) = 1
		GRBLinExpr cost_p1_opt = vars[0] + vars[2]/2 + vars[4]/3;
		GRBLinExpr cost_p2_opt = vars[1] + vars[2]/2 + vars[4]/3 + vars[5]/2;
		GRBLinExpr cost_p3_opt = vars[3] + vars[4]/3 + vars[5]/2;
		model.addConstr(cost_p1_opt + cost_p2_opt + cost_p3_opt == 1);

		/////
		// 3.
		/////

		// Add constraint that cost(opt) <= cost(Nash)
		model.addConstr(cost_p1_opt + cost_p2_opt + cost_p3_opt <= cost_p1 + cost_p2 + cost_p3);

		//////////////////////////////////////////////////
		// Set Objective: PoS has to be as big as possible
		//////////////////////////////////////////////////

		// PoS = cost(best_Nash) / cost(opt) = cost(best_Nash) bc cost(opt) == 1 (constraint)
		// PROBLEM: our Nash isn't the BEST Nash :(
		// PROBLEM 2: opt should not be Nash!!! Else we simply have PoS == 1
		model.setObjective( cost_p1 + cost_p2 + cost_p3 , GRB_MAXIMIZE);

		// Optimize model

		model.optimize();

		for (int i = 0; i < 8; i++) {
			cout << vars[i].get(GRB_StringAttr_VarName) << " "
		     << vars[i].get(GRB_DoubleAttr_X) << endl;
		}

		cout << "Something like(!) PoS ( cost(Nash)/cost(opt) ): " << model.get(GRB_DoubleAttr_ObjVal) << endl;

		delete[] vars;

	} catch(GRBException e) {
		cout << "Error code = " << e.getErrorCode() << endl;
		cout << e.getMessage() << endl;
	} catch(...) {
		cout << "Exception during optimization" << endl;
	}

	return 0;
}