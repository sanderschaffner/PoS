#include "gurobi_c++.h"
using namespace std;

int main(int argc, char *argv[]) {
	try {
		GRBEnv env = GRBEnv();

		GRBModel model = GRBModel(env);

		// Create variables

		GRBVar x1 = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_INTEGER, "x1");
		GRBVar x2 = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_INTEGER, "x2");

		// Integrate new variables

		model.update();

		// Set objective: maximize 2 x1 + x2

		model.setObjective(2 * x1 + x2, GRB_MAXIMIZE);

		// Add constraint: x2 + 0.1 x1 <= 8

		model.addConstr(x2 + 0.1 * x1 <= 8, "c0");

		// Add constraint: x2 + 5 x1 <= 70

		model.addConstr(x2 + 5 * x1 <= 70, "c1");

		// Optimize model

		model.optimize();

		cout << x1.get(GRB_StringAttr_VarName) << " "
		     << x1.get(GRB_DoubleAttr_X) << endl;
		cout << x2.get(GRB_StringAttr_VarName) << " "
		     << x2.get(GRB_DoubleAttr_X) << endl;

		cout << "Obj: " << model.get(GRB_DoubleAttr_ObjVal) << endl;

	} catch(GRBException e) {
		cout << "Error code = " << e.getErrorCode() << endl;
		cout << e.getMessage() << endl;
	} catch(...) {
		cout << "Exception during optimization" << endl;
	}

	return 0;
}