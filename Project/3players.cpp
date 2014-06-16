#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator> // for ostream_iterator
#include <string.h> // memset

#include <stdio.h>      /* printf, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

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

// globals for rec:
int profile_path[3][125]; // connects profile and path -> pp[i][x] = y: for profile x we used path y of player i
vector< vector<int> >  paths[3];
double one = 1;

void rec(int pr, bool dont_proceed, const int mult_paths, GRBModel& model, GRBVar* v_edges, const int used_profile[125][6][6], const vector<int>& which_guy, const vector<int>& which_strategy, const vector<int>& heavy_profile) {
	if (pr == mult_paths) {
		// If we reach this point, we have a new PoS:)
		//dont_proceed = false;

		model.optimize();
		for (int i = 0; i < 8; i++) {
			cout << v_edges[i].get(GRB_StringAttr_VarName) << " " << v_edges[i].get(GRB_DoubleAttr_X) << endl;
		}
		cout << "PoS: " << model.get(GRB_DoubleAttr_ObjVal) << endl;

		return;
	}

	// profile pr is not a Nash equilibrium
	// one of the guys wants to change the strategy
	// cout<< pr< < "\n";
	int i,j,k;
	int used[6][6];
	memset(used,0,sizeof(used));
	double coef[8];
	GRBLinExpr path;
	//int rr = rand()%2;
	//if (heavy_profile[pr] && rr){
	if (heavy_profile[pr]){
		memset(coef, 0, sizeof(coef));
		coef[edge(0,3)] = coef[edge(1,4)] = coef[edge(2,5)] = 1; // start with nash
		for (i=0;i<6;i++){
			for (j=i+1;j<6;j++){
				if (used_profile[pr][i][j]>0) coef[edge(i,j)]-=1;
			}
		}
		// print profile and coeffs:
/*		cout<<pr<<"\n";
		for (int ii=0;ii<6;ii++){
			for (int jj=0;jj<6;jj++)
				cout<<used_profile[pr][ii][jj]<<" ";
			cout<<"\n";
		}
		for (int jj=0;jj<8;jj++)
			cout<<coef[jj]<<" ";
		cout<<"\n";*/
		for (k=0;k<8;k++){
			if(coef[k]!=0) path = path + coef[k]*v_edges[k];
		}
		// Constraint tells: |N|<|S_i|
		GRBConstr constr = model.addConstr(path <= 0);
		model.optimize();
/*		for (int i = 0; i < 8; i++) {
			cout << v_edges[i].get(GRB_StringAttr_VarName) << " "
		     << v_edges[i].get(GRB_DoubleAttr_X) << endl;
		}
		cout << "Something like(!) PoS (=cost(Nash)/cost(opt)). Yet not minimum Nash: " << model.get(GRB_DoubleAttr_ObjVal) << endl;*/

		//if (dont_proceed) return;
		int optimstatus = model.get(GRB_IntAttr_Status);
		if (optimstatus != GRB_INF_OR_UNBD && optimstatus != GRB_INFEASIBLE) {
			if (model.get(GRB_DoubleAttr_ObjVal)>1.574) {
				if(pr==44){
					rec(104, dont_proceed, mult_paths, model, v_edges, used_profile, which_guy, which_strategy, heavy_profile);
				} else if(pr==104){
					rec(1, dont_proceed, mult_paths, model, v_edges, used_profile, which_guy, which_strategy, heavy_profile);
				} else if(pr==43){
					rec(45, dont_proceed, mult_paths, model, v_edges, used_profile, which_guy, which_strategy, heavy_profile);
				} else if(pr==103){
					rec(105, dont_proceed, mult_paths, model, v_edges, used_profile, which_guy, which_strategy, heavy_profile);
				} else {
					rec(pr+1, dont_proceed, mult_paths, model, v_edges, used_profile, which_guy, which_strategy, heavy_profile);
				}
			}				
		}
		//if (dont_proceed) return;
		// delete added constr:
		model.remove(constr);
		return;
	}
	int count = 0;
	vector<int> tmp;
	vector<int> tmp2;
	for(i = 0; i<which_guy.size(); i+=2){
		if(which_guy[i]==pr) {
			//cout<<which_strategy[i+1]<<endl;
			count++;
			tmp.push_back(which_guy[i+1]);
			tmp2.push_back(which_strategy[i+1]);
		}
	}
	srand (time(NULL));
	//int tt = rand()%count;// i: seed one each time
	//int a[2] = {rand()%count,rand()%count}; // ii: seed 2 each time
	//for (int n=0;n<2;n++){
	//	int tt = a[n];
	for (int tt=0;tt<tmp.size();tt++){ // iii: seed all

		i = tmp[tt]; // guy
		j = tmp2[tt]; // strategy
		//cout<<pr<<"  "<<i<<"  "<<j<<"\n"; 
		for (int ii=0;ii<6;ii++)
			for (int jj=0;jj<6;jj++)
				used[ii][jj] = used_profile[pr][ii][jj];
		int t = profile_path[i][pr]; // path of player i belonging to this profile (so the old one which we want to change)
		memset(coef, 0, sizeof(coef));
		//now subtract from used pp[i][pr] strategy and insert j-th strategy instead
/*		cout<<"start: "<<endl;
		for (int i = 0; i < paths[3].size(); i++) {
			vector<int>tmp = paths[3][i];
			for(int j = 0; j < tmp.size(); j++) {
				std::cout << tmp[j];
			}
			printf("\n");
	    }
		cout<<"guy "<<i<<"; path "<<t<<"; "<<endl;*/
		for (k=0;k<paths[i][t].size()-1;k++){
			coef[edge(paths[i][t][k],paths[i][t][k+1])] = -one/used[paths[i][t][k]][paths[i][t][k+1]];
			used[paths[i][t][k]][paths[i][t][k+1]]--;
			used[paths[i][t][k+1]][paths[i][t][k]]--;
		}
		//and now insert the new 
		for (k=0;k<paths[i][j].size()-1;k++){
			used[paths[i][j][k]][paths[i][j][k+1]]++;
			used[paths[i][j][k+1]][paths[i][j][k]]++;
			coef[edge(paths[i][j][k],paths[i][j][k+1])] += one/used[paths[i][j][k]][paths[i][j][k+1]];					
		}
		GRBLinExpr path2;
		for (k=0;k<8;k++){
			if(coef[k]!=0) path2 = path2 + v_edges[k]*coef[k];
		}
		//cout<<"\n";
		GRBConstr constr = model.addConstr(path2 <= 0);
		//cout<<pr<<"   "<<constraint<<" \n\n";
		model.optimize();
/*		if(model.get(GRB_DoubleAttr_ObjVal)>1.63) {
			for (int i = 0; i < 8; i++) {
				cout << v_edges[i].get(GRB_StringAttr_VarName) << " " << v_edges[i].get(GRB_DoubleAttr_X) << endl;
			}
			cout << "Something like(!) PoS (=cost(Nash)/cost(opt)). Yet not minimum Nash: " << model.get(GRB_DoubleAttr_ObjVal) << endl;
		}
		cout<<pr<<"  :)  "<<model.get(GRB_DoubleAttr_ObjVal)<<"\n";*/
		//if (dont_proceed) return;
		int optimstatus = model.get(GRB_IntAttr_Status);
		if (optimstatus != GRB_INF_OR_UNBD && optimstatus != GRB_INFEASIBLE) {
			cout << pr << " " << model.get(GRB_DoubleAttr_ObjVal) << endl;
			if (model.get(GRB_DoubleAttr_ObjVal)>1.574) { //1.574
				if(pr==44){
					rec(104, dont_proceed, mult_paths, model, v_edges, used_profile, which_guy, which_strategy, heavy_profile);
				} else if(pr==104){
					rec(1, dont_proceed, mult_paths, model, v_edges, used_profile, which_guy, which_strategy, heavy_profile);
				} else if(pr==43){
					rec(45, dont_proceed, mult_paths, model, v_edges, used_profile, which_guy, which_strategy, heavy_profile);
				} else if(pr==103){
					rec(105, dont_proceed, mult_paths, model, v_edges, used_profile, which_guy, which_strategy, heavy_profile);
				} else {
					rec(pr+1, dont_proceed, mult_paths, model, v_edges, used_profile, which_guy, which_strategy, heavy_profile);
				}
			}	
		}
		//if (dont_proceed) return;
		// delete added constr:
		model.remove(constr);
	} // for ii and iii
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
		
		paths[0] = paths_p1.getExistingPaths();
		paths[1] = paths_p2.getExistingPaths();
		paths[2] = paths_p3.getExistingPaths();

		//////////////////////////////////////////
		// Define optimization problem with GUROBI
		//////////////////////////////////////////

		GRBEnv env = GRBEnv();

		GRBModel model = GRBModel(env);

		model.getEnv().set(GRB_IntParam_OutputFlag, 0);
		
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
		int used[size][size];
		memset(used, 0, sizeof(used));
		used[0][3] = used[3][0] = used[1][4] = used[4][1] = used[2][5] = used[5][2] = 1; // used paths in nash
		double tmp_double;
		// go trough all players:
		for (int i = 0; i < 3; i++){
			// consider each path for this player:
			// !! attention: we start with j==1 since we know that in paths[i][0][k] we have the nash_path
			// !! this is true here since we DECIDED that the direct way from source to target shall be our nash_path
			// !! and the class Paths finds us this path first (if it exists in graph!)
			for (int j = 1; j < paths[i].size(); j++){
				cout << edge(i,i+3) << "    <    ";
				GRBLinExpr temp_expr;
				// go trough the edges of the path:
				for (int k = 0; k < paths[i][j].size()-1; k++){
					//edge we consider is paths[i][j][k],paths[i][j][k+1]
					tmp_double = one/(used[paths[i][j][k]][paths[i][j][k+1]]+1);
					temp_expr = temp_expr + tmp_double*v_edges[edge(paths[i][j][k],paths[i][j][k+1])];
					cout << "  +  " << one/(used[paths[i][j][k]][paths[i][j][k+1]]+1) << " * " << edge(paths[i][j][k],paths[i][j][k+1]);
				}
				// add constraint: nesh_path < all possible alternatives
				model.addConstr(v_edges[edge(i,i+3)] <= temp_expr);
				cout<<"\n";
				
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
					/*if (i0==0 && i1==0 && i2==4){
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
		/*cout << "Expensive profiles: ";
		for(int i = 0; i < expensive.size(); i++)
		{
			cout << expensive[i] <<" ";
		}
		cout << endl;
		cout << "profile_path[x]: ";
		for(int i = 0; i < 125; i++)
		{
			cout << profile_path[0][i] <<" ";
		}
		cout << endl;*/

		// 6:
		// Till now we have that |N| is Nash and direct line is min. But |N| is not yet min |N|
		// We have to go trough all possible profiles and ensure that they are either bigger than |N| or are NOT Nash
		// Therefore: Solve lp for each profile and preceed according to two situations:
		// i: |S_i| >= |N| -> We are already happy -> Save result, proceed to next S_i
		// ii: |S_i| < |N| -> Constraint is violated! |N| should be smallest Nash -> Add constraint s.t. one player wants to deviate and proceed recursively
		// Goal: We want to constrain the lp s.t. |N| is smallest Nash and all other |S_i| are either bigger than |N| or NOT Nash. We therefore maximize |N|

		// set objective: maximize Nash (yet not minimal Nash):
		model.setObjective( v_edges[edge(0,3)] + v_edges[edge(1,4)] + v_edges[edge(2,5)] , GRB_MAXIMIZE);

		// Optimize model and look at result -> not yet min Nash
		/*model.optimize();
		for (int i = 0; i < n_variables; i++) {
			cout << v_edges[i].get(GRB_StringAttr_VarName) << " "
		     << v_edges[i].get(GRB_DoubleAttr_X) << endl;
		}
		cout << "Something like(!) PoS (=cost(Nash)/cost(opt)). Yet not minimum Nash: " << model.get(GRB_DoubleAttr_ObjVal) << endl;*/

		// 6.1:
		// learning from paper example:
		// go trough all profiles and decide if it is worth to have a closer look according to the exaple out of the paper
		// if in the new strategie the costs increas according to paper_costs (an therefore diff is pos) we won't consider it
		const double paper_costs[8] = {113,277,418,318,0,549,556,664}; // These are the edge-costs from the paper -> PoS = 1.571
		const double paper_nash = paper_costs[edge(0,3)] + paper_costs[edge(1,4)] + paper_costs[edge(2,5)];
		double diff = 0; // difference 
		int pr; // profile number which we are looking at
		vector<int> which_guy; // first profile, than guy
		vector<int> which_strategy; // first profile, than strategy
		vector<int> heavy_profile(mult_paths,0);
		// go trough all profiles
		for (pr = 1; pr < mult_paths+1; pr++){
			// go trough all players
			for (int i = 0; i < 3; i++){
				// i-th guy wants to deviate
				for (int j = 0; j < paths_pp[i]; j++){
					// to the j-th strategy
					// therefore the path used in this profile (== profile_path[i][pr]) should not be j itself (else we have no change!)
					if (j != profile_path[i][pr]){
						// go trough whole used[][] and make a copy of the previous used_profile we already have computed
						for (int ii = 0; ii < size; ii++)
							for (int jj = 0; jj < size; jj++)
								used[ii][jj] = used_profile[pr][ii][jj];
						int t = profile_path[i][pr]; // path of i-th player in strategy pr which was considered and shall be changed!
						diff = 0;
						// now subtract from used t = profile_path[i][pr] (old) strategy and insert j-th (new) strategy instead
						for (k = 0; k < paths[i][t].size() - 1; k++){
							// subtract from diff all edge-costs of path t and weight it apropriate
							diff -= paper_costs[edge(paths[i][t][k],paths[i][t][k+1])] / used[paths[i][t][k]][paths[i][t][k+1]];
							// get ridd of path t in used
							used[paths[i][t][k]][paths[i][t][k+1]]--;
							used[paths[i][t][k+1]][paths[i][t][k]]--;
						}
						/*if (pr==31){
							cout << "Diff of profile = " << pr << " before adding: " << diff << endl;
						}*/
						// and now insert the new 
						for (k = 0; k < paths[i][j].size() - 1; k++){
							// FIRST update used
							used[paths[i][j][k]][paths[i][j][k+1]]++;
							used[paths[i][j][k+1]][paths[i][j][k]]++;
							// and then add costs
							diff += paper_costs[edge(paths[i][j][k],paths[i][j][k+1])] / used[paths[i][j][k]][paths[i][j][k+1]];					
						}

						// if difference is negative we save person and profile number! 
						if (diff < 0){
							if (pr > 0)
								//cout << "Profile = " << pr << ".  " << i << " guy wants to change to " << j << "-th strategy  " << "and diff is " << diff << endl;
							which_guy.push_back(pr);
							which_guy.push_back(i);
							which_strategy.push_back(pr);
							which_strategy.push_back(j);
						}					
					}
				}
			}

			diff = paper_nash;
			// go trough used_profile which has not changed!
			for (int i = 0; i < size; i++)
				for (int j = i + 1; j < size; j++){
					// subtract all costs of the proposed profile from Nash
					if (used_profile[pr][i][j] > 0)
						diff -= paper_costs[edge(i,j)];
				}
			if ( diff < 0) 
			{
				// Nash is smaller than proposed edges of profile
				//cout << pr <<"-th strategy profile is sligtly heavier than Nash and the difference is "<< diff << endl;
				heavy_profile[pr] = 1;
			}
		}
		cout << "We found " << which_guy.size()/2 << " changes we want to have a closer look on" << endl;
/*		for(int i = 0; i<which_strategy.size(); i+=2){
			cout << which_strategy[i] << " " << which_strategy[i+1] << endl;
		}*/

		// 6.2
		// Add constraints and solve each time lp. Do this recurivly and use data from which we learnd so far:
		bool dont_proceed = false;
		cout<<"Start recursion\n";
		rec(44, dont_proceed, mult_paths, model, v_edges, used_profile, which_guy, which_strategy, heavy_profile);
		cout<<"End recursion\n";
/*		for (int i = 0; i < n_variables; i++) {
			cout << v_edges[i].get(GRB_StringAttr_VarName) << " "
		     << v_edges[i].get(GRB_DoubleAttr_X) << endl;
		}
		cout << "Something like(!) PoS (=cost(Nash)/cost(opt)). Yet not minimum Nash: " << model.get(GRB_DoubleAttr_ObjVal) << endl;*/

	} catch(GRBException e) {
		cout << "Error code = " << e.getErrorCode() << endl;
		cout << e.getMessage() << endl;
	} catch(...) {
		cout << "Exception during optimization" << endl;
	}

	return 0;
}