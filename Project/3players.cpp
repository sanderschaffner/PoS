#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator> // for ostream_iterator
#include <string.h> // memset

#include "gurobi_c++.h"
/*#include "matrix.h"*/
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
	else if (i==3 && j==5)
		return 4;
	else if (i==0 && j==3)
		return 5;
	else if (i==1 && j==4)
		return 6;
	else if (i==2 && j==5)
		return 7;
	else
		cout << "Edge not defined! i = " << i << ", j = " << j << endl;
		return -1; // gives segfault bc array[-1] doesnt exist
}

int main(int argc, char *argv[]) {
	try {
		////////////////////
		// Variables to set:
		////////////////////
		const unsigned int size = 6; // defines how many nodes we have in our graph (also edit this in paths.h)
		const int n_variables = 8; // defines the number of edges we have in our graph
		const double lowerBound = 0.001; // lower bound for Gurobi
		const double upperBound = GRB_INFINITY; // upper bound for Gurobi

		///////////////////////////////////////////////////////////
		// Create Graph and find all possible paths for each player
		///////////////////////////////////////////////////////////

		// 1:
		// Matrix of graph we want to get PoS
/*		Matrix<bool> graph(6,6);
		graph(0,1) = 1; graph(1,2) = 1; graph(2,3) = 1; graph(3,4) = 1; graph(4,5) = 1;
		graph(0,3) = 1; graph(1,4) = 1; graph(2,5) = 1;
		// symmetric:
		graph(1,0) = 1; graph(2,1) = 1; graph(3,2) = 1; graph(4,3) = 1; graph(5,4) = 1;
		graph(3,0) = 1; graph(4,1) = 1; graph(5,2) = 1;*/
		//cout << "matrix: " << endl << graph << endl;
		unsigned int graph[size][size];
		memset(graph, 0, sizeof(graph));
		graph[0][1] = 1; graph[1][2] = 1; graph[2][4] = 1; graph[3][4] = 1; graph[3][5] = 1;
		graph[1][0] = 1; graph[2][1] = 1; graph[4][2] = 1; graph[4][3] = 1; graph[5][3] = 1;
		graph[0][3] = 1; graph[1][4] = 1; graph[2][5] = 1;
		graph[3][0] = 1; graph[4][1] = 1; graph[5][2] = 1;

		// 2:
		// Get all possible paths for each player in the graph
		Paths paths_p1(0,3); // source, target
		Paths paths_p2(1,4); // source, target
		Paths paths_p3(2,5); // source, target
		paths_p1.getAllPaths(graph);
		paths_p2.getAllPaths(graph);
		paths_p3.getAllPaths(graph);
		//paths_p2.print();
		vector< vector<int> >  paths[3];
		paths[0] = paths_p1.getExistingPaths();
		paths[1] = paths_p2.getExistingPaths();
		paths[2] = paths_p3.getExistingPaths();

		//////////////////////////////////////////
		// Define optimization problem with GUROBI
		//////////////////////////////////////////

		GRBEnv env = GRBEnv();

		GRBModel model = GRBModel(env);
		
		// Create variables
		// var_number: note x -> node y
		// short, direct ones: 0: 0<->1, 1: 1<->2, 2: 2<->4, 3: 3<->4, 4: 3<->5
		// direct from source to target: 5: 0<->3, 6: 1<->4, 7: 2<->5
		double lb[n_variables];
		fill_n(lb, n_variables, lowerBound);
		double ub[n_variables];
		fill_n(ub, n_variables, upperBound);
		GRBVar* v_edges = model.addVars(lb, ub, NULL, NULL, NULL, n_variables);
		model.update();

		// 3:
		// Inequalities saying that the minimum spanning tree is the line (optimum=={0to1to2to4to3to5})
		// So direct source to target paths have to be bigger than short ones
		model.addConstr(v_edges[edge(0,1)] <= v_edges[edge(0,3)]);
		model.addConstr(v_edges[edge(1,2)] <= v_edges[edge(0,3)]);
		model.addConstr(v_edges[edge(2,4)] <= v_edges[edge(0,3)]);
		model.addConstr(v_edges[edge(3,4)] <= v_edges[edge(0,3)]);
		model.addConstr(v_edges[edge(1,2)] <= v_edges[edge(1,4)]);
		model.addConstr(v_edges[edge(2,4)] <= v_edges[edge(1,4)]);
		model.addConstr(v_edges[edge(2,4)] <= v_edges[edge(2,5)]);
		model.addConstr(v_edges[edge(3,4)] <= v_edges[edge(2,5)]);
		model.addConstr(v_edges[edge(3,5)] <= v_edges[edge(2,5)]);
		// Cost of the spanning tree is 1 (optimum has cost one -> result of lp gives directly PoS)
		GRBLinExpr cost_p1_opt = v_edges[edge(0,1)] + v_edges[edge(1,2)]/2 + v_edges[edge(2,4)]/3;
		GRBLinExpr cost_p2_opt = v_edges[edge(1,2)]/2 + v_edges[edge(2,4)]/3 + v_edges[edge(3,4)]/2;
		GRBLinExpr cost_p3_opt = v_edges[edge(2,4)]/3 + v_edges[edge(3,4)]/2 + v_edges[edge(3,5)];
		model.addConstr(cost_p1_opt + cost_p2_opt + cost_p3_opt == 1); // equivalent to: c1+...+c5==1

		// 4:
		// Inequalities checking that the edges (0,3) (1,4) and (2,5) give a Nash equilibrium
		bool used[6][6];
		double one = 1;
		memset(used, 0, sizeof(used));
		used[0][3] = used[3][0] = used[1][4] = used[4][1] = used[2][5] = used[5][2] = 1; // used paths in nash
		double tmp_double;
		// go trough all players:
		for (int i = 0; i < 3; i++){
			GRBLinExpr temp_expr;
			// consider each path for this player:
			// !! attention: we start with j==1 since we know that in paths[i][0][k] we have the nash_path
			// !! this is true here since we DECIDED that the direct way from source to target shall be our nash_path
			// !! and the class Paths finds us this path first (if it exists in graph!)
			for (int j = 1; j < paths[i].size(); j++){
				//cout << edge(i,i+3) << "    <    ";
				// go trough the edges of the path:
				for (int k = 0; k < paths[i][j].size()-1; k++){
					//edge we consider is paths[i][j][k],paths[i][j][k+1]
					tmp_double = one/(used[paths[i][j][k]][paths[i][j][k+1]]+1);
					temp_expr = temp_expr + tmp_double*v_edges[edge(paths[i][j][k],paths[i][j][k+1])];
					//cout << "  +  " << one/(used[paths[i][j][k]][paths[i][j][k+1]]+1) << " * " << edge(paths[i][j][k],paths[i][j][k+1]);
				}
				// add constraint: nesh_path < all possible alternatives
				model.addConstr(v_edges[edge(i,i+3)] <= temp_expr);
				//cout<<"\n";
				
			}
		}

		// 5:
		// Create used_profile: According to all possible combinations of paths for each player,
		// we count how often each edge is visited, so that we have a look-up table for later.
		int paths_pp[3] = {paths[0].size(),paths[1].size(),paths[2].size()}; // number of paths per person
		int i0, i1, i2, k;
		int profile = 0;
		int mult_paths = paths_pp[0] * paths_pp[1] * paths_pp[2];
		int used_profile[mult_paths][size][size];
		memset(used_profile, 0, sizeof(used_profile));
		int profile_path[3][mult_paths]; // connects profile and path -> pp[i][x] = y: for profile x we used path y of player i
		vector<int> expensive;
		for (i0 = 0; i0 < paths_pp[0]; i0++){
			for (i1 = 0; i1 < paths_pp[1]; i1++){
				for (i2 = 0; i2 < paths_pp[2]; i2++){
					for (k=0;k<paths[0][i0].size()-1;k++){
						used_profile[profile][paths[0][i0][k]][paths[0][i0][k+1]]++;
						used_profile[profile][paths[0][i0][k+1]][paths[0][i0][k]]++;
					}
					for (k=0;k<paths[1][i1].size()-1;k++){
						used_profile[profile][paths[1][i1][k]][paths[1][i1][k+1]]++;
						used_profile[profile][paths[1][i1][k+1]][paths[1][i1][k]]++;
					}
					for (k=0;k<paths[2][i2].size()-1;k++){
						used_profile[profile][paths[2][i2][k]][paths[2][i2][k+1]]++;
						used_profile[profile][paths[2][i2][k+1]][paths[2][i2][k]]++;
					}
					// Print out one used_profile and the paths beloging to it
					/*if (i0==1 && i1==1 && i2==1){
						cout<<profile<<"\n";
						for (int ii=0;ii<6;ii++){
							for (int jj=0;jj<6;jj++)
								cout<<used_profile[profile][ii][jj]<<" ";
							cout<<"\n";
						}
						cout<<"path 0: ";
						for(int bl=0; bl < paths[0][i0].size();bl++ )
							cout<<paths[0][i0][bl];
						cout<<endl<<"path 1: ";
						for(int bl=0; bl < paths[1][i1].size();bl++ )
							cout<<paths[1][i1][bl];
						cout<<endl<<"path 2: ";
						for(int bl=0; bl < paths[2][i1].size();bl++ )
							cout<<paths[2][i2][bl];
						cout<<endl;
					}*/	
					profile_path[0][profile] = i0;
					profile_path[1][profile] = i1;
					profile_path[2][profile] = i2;

					// If the Nash-edges are part of the profile, we know for sure that: |S_i| >= |Nash| -> usefull later
					if (used_profile[profile][0][3]>0 && used_profile[profile][1][4]>0 && used_profile[profile][2][5]>0)
					{
						expensive.push_back(profile);
					}

					profile++;
				}
			}
		}
		cout << "Expensive profiles: ";
		for(int i = 0; i < expensive.size(); i++)
		{
			cout << expensive[i] <<" ";
		}
		cout << endl;

	} catch(GRBException e) {
		cout << "Error code = " << e.getErrorCode() << endl;
		cout << e.getMessage() << endl;
	} catch(...) {
		cout << "Exception during optimization" << endl;
	}

	return 0;
}