#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator> // for ostream_iterator
#include <string.h> // memset

#include <stdio.h>      /* printf, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

#include "gurobi_c++.h"
#include "paths.h"

using namespace std;

//without eps
//highest nesh: wrong: [0,1,0.136364],[1,2,0.237013],[2,4,0.37013],[4,3,0.256494],[3,5,0],[0,3,0.506494],[1,4,0.493506],[2,5,0.626623]
//mult by 1000000: [0,1,136364],[1,2,237013],[2,4,370130],[4,3,256494],[3,5,0],[0,3,506494],[1,4,493506],[2,5,626623]

//lower nesh: wrong: [0,1,0.0997831],[1,2,0.247289],[2,4,0.370933],[4,3,0.281996],[3,5,0],[0,3,0.488069],[1,4,0.494577],[2,5,0.591106]
//[0,3],[1,4],[2,5]

//with eps
// 1.573749: [0,1,0.0997837],[1,2,0.247287],[2,4,0.370934],[4,3,0.281996],[3,5,0],[0,3,0.488069],[1,4,0.494576],[2,5,0.591104]

//[0,1,],[1,2,],[2,4,],[4,3,],[3,5,],[0,3,],[1,4,],[2,5,]
//[0,1,0.],[1,2,0.],[2,4,0.],[4,3,0.],[3,5,0.],[0,3,0.],[1,4,0.],[2,5,0.]

typedef long int constrVar; // constraint type

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
	else if (i==2 && j==3)
		return 2;
	else if (i==3 && j==4)
		return 3;
	else if (i==0 && j==4)
		return 4;
	else if (i==1 && j==3)
		return 5;
	else if (i==2 && j==4)
		return 6;
	else
		cout << "Edge not defined! i = " << i << ", j = " << j << endl;
		return -1; // gives segfault bc array[-1] doesnt exist
}

// globals for learning:)
int rec_count=0;
int opt_count=0;
int number_of_zeros=0;
constrVar opt_before = 0;
int biggest_nr=0;

void rec(int number, const vector<unsigned int>& profileOrder, const unsigned int& n_variables, const unsigned int& size, const int& mult_paths, GRBModel& model, GRBVar* v_edges, const vector< vector< vector< int > > >& used_profile, const vector<int>& which_guy, const vector<int>& which_strategy, vector<int>& heavy_profile, constrVar& maximum, const vector< vector< vector< int > > >& paths, const vector< vector< int > >& profile_path, const constrVar& one, const constrVar& eps, const unsigned int& alternative_paths, const bool learning, vector< vector < vector <int> > >& profile_path_memory) {
	rec_count++;
	int pr = profileOrder[number];
	if(number>biggest_nr) biggest_nr=number;
	if(!(rec_count%10000)) {
		cout<<"heavy: ";
		for (int pr = 0; pr < mult_paths; pr++){
			if(heavy_profile[pr]) cout<<pr<<" ";
		}
		cout<<endl;
		int temp_count = 0;
		int temp_heavy = 0;
		for (int pr = 0; pr < mult_paths; pr++){
			// go trough all players
			for (int i = 0; i < 3; i++){
				// i-th guy wants to deviate
				for (int j = 0; j < profile_path_memory[pr][i].size(); j++){
					if(profile_path_memory[pr][i][j]==1) temp_count++;
				}
			}
		}
		for (int pr = 0; pr < mult_paths; pr++){
			temp_heavy += heavy_profile[pr];
		}
		cout<<rec_count<<" We have "<<temp_count<<" ones in profile_path_memory and the zero count is at "<<number_of_zeros<<" opt count: "<<opt_count<<" heavy count: "<<temp_heavy<<" profile: "<<number<<" latest pr: "<<profileOrder[biggest_nr]<<endl;
	}

	if (pr == mult_paths) {
		// If we reach this point, we have a new PoS:)

		model.optimize();
		for (int i = 0; i < n_variables; i++) {
			cout << v_edges[i].get(GRB_StringAttr_VarName) << " " << v_edges[i].get(GRB_DoubleAttr_X) << endl;
		}
		cout << "for python script: [0,1,"<<v_edges[0].get(GRB_DoubleAttr_X)<<
				"],[1,2,"<<v_edges[1].get(GRB_DoubleAttr_X)<<
				"],[2,3,"<<v_edges[2].get(GRB_DoubleAttr_X)<<
				"],[3,4,"<<v_edges[3].get(GRB_DoubleAttr_X)<<
				"],[0,4,"<<v_edges[4].get(GRB_DoubleAttr_X)<<
				"],[1,3,"<<v_edges[5].get(GRB_DoubleAttr_X)<<
				"],[2,4,"<<v_edges[6].get(GRB_DoubleAttr_X)<<
				"]"<<endl;

		maximum = model.get(GRB_DoubleAttr_ObjVal);
		cout << "PoS: " << model.get(GRB_DoubleAttr_ObjVal) << endl;

		return;
	}

	// profile pr is not a Nash equilibrium
	// one of the guys wants to change the strategy
	// cout<< pr< < "\n";
	int i,j,k;
	int used[size][size];
	memset(used,0,sizeof(used));
	constrVar coef[n_variables];
	GRBLinExpr path;

	// only valid for learning:
	if (heavy_profile[pr]){
		memset(coef, 0, sizeof(coef));
		coef[edge(0,4)] = coef[edge(1,3)] = coef[edge(2,4)] = 1; // start with nash
		for (i=0;i<size;i++){
			for (j=i+1;j<size;j++){
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
		for (k=0;k<n_variables;k++){
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

		int optimstatus = model.get(GRB_IntAttr_Status);
		if (optimstatus != GRB_INF_OR_UNBD && optimstatus != GRB_INFEASIBLE) {
/*			if(opt_before == model.get(GRB_DoubleAttr_ObjVal)) {
				profile_path_memory[pr][i][j] = 0;
				number_of_zeros++;
			}*/
			opt_before = model.get(GRB_DoubleAttr_ObjVal);
			//cout << pr << " " << model.get(GRB_DoubleAttr_ObjVal) << " " << maximum << endl;
			if (model.get(GRB_DoubleAttr_ObjVal)>maximum) {
				rec(number+1, profileOrder, n_variables, size, mult_paths, model, v_edges, used_profile, which_guy, which_strategy, heavy_profile, maximum, paths, profile_path, one, eps, alternative_paths, learning, profile_path_memory);
			}				
		}
		// delete added constr:
		model.remove(constr);
		return;
	}

	// 2 Cases:
	// i: profile > nash
	// ii: player x wants to deviate from i to j

	if( !learning ) {
		// CASE I
		// profile > nash

		memset(coef, 0, sizeof(coef));
		coef[edge(0,4)] = coef[edge(1,3)] = coef[edge(2,4)] = 1; // start with nash
		for (i=0;i<size;i++){
			for (j=i+1;j<size;j++){
				if (used_profile[pr][i][j]>0) coef[edge(i,j)]-=1;
			}
		}
		for (k=0;k<n_variables;k++){
			if(coef[k]!=0) path = path + coef[k]*v_edges[k];
		}
		// Constraint tells: |N|<|S_i|
		GRBConstr constr = model.addConstr(path <= 0);
		if( !profile_path_memory[profile_path_memory.size()-1][0][pr] ){
			//cout<<"direct to rec from nash<prof! "<<number<<endl;
			rec(number+1, profileOrder, n_variables, size, mult_paths, model, v_edges, used_profile, which_guy, which_strategy, heavy_profile, maximum, paths, profile_path, one, eps, alternative_paths, learning, profile_path_memory);
		} else {
			model.optimize();
			opt_count++;

			int optimstatus = model.get(GRB_IntAttr_Status);
			if (optimstatus != GRB_INF_OR_UNBD && optimstatus != GRB_INFEASIBLE) {
				if(opt_before == model.get(GRB_DoubleAttr_ObjVal)) {
					profile_path_memory[profile_path_memory.size()-1][0][pr] = 0;
					number_of_zeros++;
				}
				//cout << pr << " " << maximum << endl;
				if (model.get(GRB_DoubleAttr_ObjVal)>maximum) {
					opt_before = model.get(GRB_DoubleAttr_ObjVal);
					//heavy_profile[pr] = 1;
					rec(number+1, profileOrder, n_variables, size, mult_paths, model, v_edges, used_profile, which_guy, which_strategy, heavy_profile, maximum, paths, profile_path, one, eps, alternative_paths, learning, profile_path_memory);
				}
								
			}
		}
		// delete added constr:
		model.remove(constr);
	}

	// CASE II
	// player x wants to deviate from i to j
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

	if( alternative_paths == 0 ) {
		for (int tt=0;tt<tmp.size();tt++){
			i = tmp[tt]; // guy
			j = tmp2[tt]; // strategy
			bool temp_er=true;
			for(int e = 0; e < profile_path_memory[pr][i].size(); e++){
				if(profile_path_memory[pr][i][e]) temp_er=false;
			}
			if(temp_er) {
				/*for(int c=0; c<profileOrder.size(); c++)
					if(profileOrder[c] == pr) {
						profileOrder.erase(profileOrder.begin()+c);
					}*/
				heavy_profile[pr] = 1;
				cout << "bla" << endl;
				return;
			}
			//cout<<pr<<"  "<<i<<"  "<<j<<"\n"; 
			for (int ii=0;ii<size;ii++)
				for (int jj=0;jj<size;jj++)
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
			for (k=0;k<n_variables;k++){
				if(coef[k]!=0) path2 = path2 + v_edges[k]*coef[k];
			}
			//cout<<"\n";
			GRBConstr constr2 = model.addConstr(path2 <= 0-eps);
			//cout<<pr<<"   "<<constraint<<" \n\n";
			if( !profile_path_memory[pr][i][j] ) {
				//cout<<"direct to rec from deviate! "<<number<<" opt_bef:"<<opt_before<<" max:"<<maximum<<endl;
				rec(number+1, profileOrder, n_variables, size, mult_paths, model, v_edges, used_profile, which_guy, which_strategy, heavy_profile, maximum, paths, profile_path, one, eps, alternative_paths, learning, profile_path_memory);
			} else {
				model.optimize();
				opt_count++;
		/*		if(model.get(GRB_DoubleAttr_ObjVal)>1.63) {
					for (int i = 0; i < 8; i++) {
						cout << v_edges[i].get(GRB_StringAttr_VarName) << " " << v_edges[i].get(GRB_DoubleAttr_X) << endl;
					}
					cout << "Something like(!) PoS (=cost(Nash)/cost(opt)). Yet not minimum Nash: " << model.get(GRB_DoubleAttr_ObjVal) << endl;
				}
				cout<<pr<<"  :)  "<<model.get(GRB_DoubleAttr_ObjVal)<<"\n";*/

				int optimstatus2 = model.get(GRB_IntAttr_Status);
				if (optimstatus2 != GRB_INF_OR_UNBD && optimstatus2 != GRB_INFEASIBLE) {
					//cout << pr << " obj: " << model.get(GRB_DoubleAttr_ObjVal)<< " bef: " << opt_before << " #0: " <<number_of_zeros << " max: " << maximum << endl;
					if(opt_before == model.get(GRB_DoubleAttr_ObjVal)) {
						profile_path_memory[pr][i][j] = 0;
						number_of_zeros++;
					}
					if (model.get(GRB_DoubleAttr_ObjVal)>maximum) { //1.574
						opt_before = model.get(GRB_DoubleAttr_ObjVal);
						rec(number+1, profileOrder, n_variables, size, mult_paths, model, v_edges, used_profile, which_guy, which_strategy, heavy_profile, maximum, paths, profile_path, one, eps, alternative_paths, learning, profile_path_memory);
					}	
				}
			}
			// delete added constr:
			model.remove(constr2);
		}
	} else {
		srand (time(NULL));
		std::vector<int> myvector;
		for (int numb = 0; numb < count; numb++) myvector.push_back(numb);
		int a[alternative_paths];
		int counter = count;
		for(int w = 0; w < alternative_paths; w++){
			int del = rand()%counter;
			a[w] = myvector[del];
			myvector.erase(myvector.begin()+del);
			counter--;
		}

		for (int n=0;n<alternative_paths;n++){
			int tt = a[n];
		//for (int tt=0;tt<tmp.size();tt++){ // iii: seed all
			i = tmp[tt]; // guy
			j = tmp2[tt]; // strategy
			if(!profile_path_memory[pr][i][j]) return;
			//cout<<pr<<"  "<<i<<"  "<<j<<"\n"; 
			for (int ii=0;ii<size;ii++)
				for (int jj=0;jj<size;jj++)
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
			for (k=0;k<n_variables;k++){
				if(coef[k]!=0) path2 = path2 + v_edges[k]*coef[k];
			}
			//cout<<"\n";
			GRBConstr constr2 = model.addConstr(path2 <= 0-eps);
			//cout<<pr<<"   "<<constraint<<" \n\n";
			model.optimize();
	/*		if(model.get(GRB_DoubleAttr_ObjVal)>1.63) {
				for (int i = 0; i < 8; i++) {
					cout << v_edges[i].get(GRB_StringAttr_VarName) << " " << v_edges[i].get(GRB_DoubleAttr_X) << endl;
				}
				cout << "Something like(!) PoS (=cost(Nash)/cost(opt)). Yet not minimum Nash: " << model.get(GRB_DoubleAttr_ObjVal) << endl;
			}
			cout<<pr<<"  :)  "<<model.get(GRB_DoubleAttr_ObjVal)<<"\n";*/

			int optimstatus2 = model.get(GRB_IntAttr_Status);
			if (optimstatus2 != GRB_INF_OR_UNBD && optimstatus2 != GRB_INFEASIBLE) {
				if(opt_before == model.get(GRB_DoubleAttr_ObjVal)) {
					profile_path_memory[pr][i][j] = 0;
					number_of_zeros++;
				}
				opt_before = model.get(GRB_DoubleAttr_ObjVal);
				//cout << pr << " " << model.get(GRB_DoubleAttr_ObjVal) << " " << maximum << endl;
				if (model.get(GRB_DoubleAttr_ObjVal)>maximum) { //1.574
					rec(number+1, profileOrder, n_variables, size, mult_paths, model, v_edges, used_profile, which_guy, which_strategy, heavy_profile, maximum, paths, profile_path, one, eps, alternative_paths, learning, profile_path_memory);
				}	
			}
			// delete added constr:
			model.remove(constr2);
		}
	}
}

int main(int argc, char *argv[]) {
	try {
		////////////////////
		// Variables to set:
		////////////////////
		const unsigned int size = 5; // defines how many nodes we have in our graph (also edit this in paths.h)
		const unsigned int n_variables = 7; // defines the number of edges we have in our graph
		const constrVar one = 60000;
		const constrVar eps = 1;
		const unsigned int alternative_paths = 0; // alternative paths to consider in rec: 0 -> all, 1,..,n -> random subset in each call
		const bool learning = true;
		const double learn_costs[n_variables] = {113,277,418,318,549,556,664}; // These are the edge-costs from the paper -> PoS = 1.571
		constrVar maximum = 1.57*one; // 1.57, starting point for finding PoS

		/////////////////////
		// Order of profiles:
		/////////////////////
		unsigned int numberOfChanges = 13; // number of profiles accounted for in next line
		unsigned int first[] = {40,104,19,14,11,4,1,103,15,10,16,45,69}; // write here the profiles which have to be consiered first!
		//unsigned int ignor[] = {4,6,9,12,13,15,17,18,20,21,24,25,26,27,30,31,34,35,37,38,40,42,45,46,48,49,53,54,65,68,69,70,73,74,78,79,80,81,84,85,86,89,91,93,94,96,98,99,101,102,111,112,113,114,115,116,117,120,123,124};

		///////////////////////////////////////////////////////////
		// Create Graph and find all possible paths for each player
		///////////////////////////////////////////////////////////

		// 1:
		// Matrix of graph we want to get PoS
		vector< vector< unsigned int > > graph;
		vector< unsigned int > tempZero;
		for(int i = 0; i < size; i++){
			tempZero.push_back(0);
		}
		for(int i = 0; i < size; i++){
			graph.push_back(tempZero);
		}
		//unsigned int graph[size][size];
		//memset(graph, 0, sizeof(graph));
		graph[0][1] = 1; graph[1][2] = 1; graph[2][3] = 1; graph[3][4] = 1;
		graph[1][0] = 1; graph[2][1] = 1; graph[3][2] = 1; graph[4][3] = 1;
		graph[0][4] = 1; graph[1][3] = 1; graph[2][4] = 1;
		graph[4][0] = 1; graph[3][1] = 1; graph[4][2] = 1;

		// 2:
		// Get all possible paths for each player in the graph
		Paths paths_p1(0,4,size); // source, target
		Paths paths_p2(1,3,size); // source, target
		Paths paths_p3(2,4,size); // source, target
		paths_p1.getAllPaths(graph);
		paths_p2.getAllPaths(graph);
		paths_p3.getAllPaths(graph);
		//paths_p3.print();
		
		vector< vector< vector<int> > >  paths;
		paths.push_back(paths_p1.getExistingPaths());
		paths.push_back(paths_p2.getExistingPaths());
		paths.push_back(paths_p3.getExistingPaths());

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
		GRBVar* v_edges = model.addVars(n_variables, GRB_INTEGER); // GRB_INTEGER, GRB_CONTINUOUS
		model.update();

		// 3:
		// Inequalities saying that the minimum spanning tree is the line (optimum=={0to1to2to4to3to5})
		// So direct source to target paths have to be bigger than short ones
		model.addConstr(v_edges[edge(0,1)] <= v_edges[edge(0,4)]);
		model.addConstr(v_edges[edge(1,2)] <= v_edges[edge(0,4)]);
		model.addConstr(v_edges[edge(2,3)] <= v_edges[edge(0,4)]);
		model.addConstr(v_edges[edge(3,4)] <= v_edges[edge(0,4)]);
		model.addConstr(v_edges[edge(1,2)] <= v_edges[edge(1,3)]);
		model.addConstr(v_edges[edge(2,3)] <= v_edges[edge(1,3)]);
		model.addConstr(v_edges[edge(2,3)] <= v_edges[edge(2,4)]);
		model.addConstr(v_edges[edge(3,4)] <= v_edges[edge(2,4)]);
		// Cost of the spanning tree is 1 (optimum has cost one -> result of lp gives directly PoS)
/*		GRBLinExpr cost_p1_opt = v_edges[edge(0,1)] + v_edges[edge(1,2)]/2 + v_edges[edge(2,4)]/3;
		GRBLinExpr cost_p2_opt = v_edges[edge(1,2)]/2 + v_edges[edge(2,4)]/3 + v_edges[edge(3,4)]/2;
		GRBLinExpr cost_p3_opt = v_edges[edge(2,4)]/3 + v_edges[edge(3,4)]/2 + v_edges[edge(3,5)];
		model.addConstr(cost_p1_opt + cost_p2_opt + cost_p3_opt == one); // equivalent to: c1+...+c5==1*/
		model.addConstr(v_edges[edge(0,1)] + v_edges[edge(1,2)] + v_edges[edge(2,3)] + v_edges[edge(3,4)] == one);

		// 4:
		// Inequalities checking that the edges (0,3) (1,4) and (2,5) give a Nash equilibrium
		int used[size][size];
		memset(used, 0, sizeof(used));
		used[0][4] = used[4][0] = used[1][3] = used[3][1] = used[2][4] = used[4][2] = 1; // used paths in nash
		constrVar temp_coef;
		// go trough all players:
		for (int i = 0; i < 3; i++){
			int x;
			// consider each path for this player:
			// !! attention: we start with j==1 since we know that in paths[i][0][k] we have the nash_path
			// !! this is true here since we DECIDED that the direct way from source to target shall be our nash_path
			// !! and the class Paths finds us this path first (if it exists in graph!)
			for (int j = 1; j < paths[i].size(); j++){
				if( i == 0 || i == 2 ) {
					x = 4;
				} else {
					x = 3;
				}
				cout << edge(i,x) << "    <    ";
				GRBLinExpr temp_expr;
				// go trough the edges of the path:
				for (int k = 0; k < paths[i][j].size()-1; k++){
					//edge we consider is paths[i][j][k],paths[i][j][k+1]
					temp_coef = one/(used[paths[i][j][k]][paths[i][j][k+1]]+1);
					temp_expr = temp_expr + temp_coef*v_edges[edge(paths[i][j][k],paths[i][j][k+1])];
					cout << "  +  " << one/(used[paths[i][j][k]][paths[i][j][k+1]]+1) << " * " << edge(paths[i][j][k],paths[i][j][k+1]);
				}
				// add constraint: nesh_path < all possible alternatives
				model.addConstr(v_edges[edge(i,x)] <= temp_expr);
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
		vector< vector< vector<int> > > used_profile;
		vector< vector<int> > profile_path; // connects profile and path -> pp[i][x] = y: for profile x we used path y of player i
		// initialize used_profile[mult_paths][size][size]:
		vector<int> sizeSingle;
		vector<int> multi;
		vector< vector<int> > sizeXsize;
		for(i0 = 0; i0 < size; i0++){
			sizeSingle.push_back(0);
		}
		for(i0 = 0; i0 < size; i0++){
			sizeXsize.push_back(sizeSingle);
		}
		for(i0 = 0; i0 < mult_paths; i0++){
			used_profile.push_back(sizeXsize);
			multi.push_back(0);
		}
		for(i0 = 0; i0 < 3; i0++){
			profile_path.push_back(multi);
		}
		//int used_profile[mult_paths][size][size];
		//memset(used_profile, 0, sizeof(used_profile));
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
/*					if (i0==0 && i1==3 && i2==4){
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
						for(int bl=0; bl < paths[2][i2].size();bl++ )
							cout<<paths[2][i2][bl];
						cout<<endl;
					}*/	
					profile_path[0][profile] = i0;
					profile_path[1][profile] = i1;
					profile_path[2][profile] = i2;

					// If the Nash-edges are part of the profile, we know for sure that: |S_i| >= |Nash| -> usefull later
					if (used_profile[profile][0][4]>0 && used_profile[profile][1][3]>0 && used_profile[profile][2][4]>0)
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

		// 5.1: create order of profiles:
		vector< unsigned int > profileOrder;
		bool notIn;
		for(int i = 0; i < numberOfChanges; i++){
			profileOrder.push_back(first[i]);
		}		
		for(unsigned int i = 1; i<mult_paths+1; i++){
			notIn = true;
			for(int j = 0; j < numberOfChanges; j++){
				if(i == first[j]) notIn = false;
			}
			for(int j = 0; j < expensive.size(); j++){
				if(i == expensive[j]) notIn = false;
			}
			if( notIn ) profileOrder.push_back(i);
		}
		cout << "We go trough " << profileOrder.size()-1 << " profiles and consider " << alternative_paths << " alternative paths in each call (0 == all)" << endl;
		cout << "Learning is " << learning << endl;
		cout << "expensive:" << endl;
		for(int i = 0; i < expensive.size(); i++){
			cout<< expensive[i]<<" ";
		}
		cout << endl;
		cout << mult_paths << endl;
		for(int i = 0; i < profileOrder.size(); i++){
			cout<< profileOrder[i]<<" ";
		}
		cout << endl;

		// 6:
		// Till now we have that |N| is Nash and direct line is min. But |N| is not yet min |N|
		// We have to go trough all possible profiles and ensure that they are either bigger than |N| or are NOT Nash
		// Therefore: Solve lp for each profile and preceed according to two situations:
		// i: |S_i| >= |N| -> We are already happy -> Save result, proceed to next S_i
		// ii: |S_i| < |N| -> Constraint is violated! |N| should be smallest Nash -> Add constraint s.t. one player wants to deviate and proceed recursively
		// Goal: We want to constrain the lp s.t. |N| is smallest Nash and all other |S_i| are either bigger than |N| or NOT Nash. We therefore maximize |N|

		// set objective: maximize Nash (yet not minimal Nash):
		model.setObjective( v_edges[edge(0,4)] + v_edges[edge(1,3)] + v_edges[edge(2,4)] , GRB_MAXIMIZE);

		// Optimize model and look at result -> not yet min Nash
		/*model.optimize();
		for (int i = 0; i < n_variables; i++) {
			cout << v_edges[i].get(GRB_StringAttr_VarName) << " "
		     << v_edges[i].get(GRB_DoubleAttr_X) << endl;
		}
		cout << "Something like(!) PoS (=cost(Nash)/cost(opt)). Yet not minimum Nash: " << model.get(GRB_DoubleAttr_ObjVal) << endl;*/

		// 6.0
		// Prepare vector to remember if a certain change of path in a profile is worth trying again
		vector< vector < vector <int> > > profile_path_memory;
		vector<int> temp1;
		vector< vector<int> > temp2;
		for (int pr = 0; pr < mult_paths; pr++){
			temp2.clear();
			// go trough all players
			for (int i = 0; i < 3; i++){
				temp1.clear();
				// i-th guy wants to deviate
				for (int j = 0; j < paths_pp[i]; j++){
					temp1.push_back(1);
				}
				temp2.push_back(temp1);
			}
			profile_path_memory.push_back(temp2);
		}
		temp1.clear();
		temp2.clear();
		for (int pr = 0; pr < mult_paths; pr++){
			temp1.push_back(1);
		}
		temp2.push_back(temp1);
		profile_path_memory.push_back(temp2);

		// 6.1:
		// learning from paper example:
		// go trough all profiles and decide if it is worth to have a closer look according to the exaple out of the paper
		// if in the new strategie the costs increas according to learn_costs (an therefore diff is pos) we won't consider it
		const double paper_nash = learn_costs[edge(0,4)] + learn_costs[edge(1,3)] + learn_costs[edge(2,4)];
		double diff = 0; // difference 
		int pr; // profile number which we are looking at
		vector<int> which_guy; // first profile, than guy
		vector<int> which_strategy; // first profile, than strategy
		vector<int> heavy_profile(mult_paths,0);
		// go trough all profiles
		for (pr = 1; pr < mult_paths; pr++){
			// go trough all players
			for (int i = 0; i < 3; i++){
				// i-th guy wants to deviate
				for (int j = 0; j < paths_pp[i]; j++){
					if(learning) {
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
								diff -= learn_costs[edge(paths[i][t][k],paths[i][t][k+1])] / used[paths[i][t][k]][paths[i][t][k+1]];
								// get ridd of path t in used
								used[paths[i][t][k]][paths[i][t][k+1]]--;
								used[paths[i][t][k+1]][paths[i][t][k]]--;
							}
							//if (pr==31){
							//	cout << "Diff of profile = " << pr << " before adding: " << diff << endl;
							//}
							// and now insert the new 
							for (k = 0; k < paths[i][j].size() - 1; k++){
								// FIRST update used
								used[paths[i][j][k]][paths[i][j][k+1]]++;
								used[paths[i][j][k+1]][paths[i][j][k]]++;
								// and then add costs
								diff += learn_costs[edge(paths[i][j][k],paths[i][j][k+1])] / used[paths[i][j][k]][paths[i][j][k+1]];					
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
					} else {
						if (j != profile_path[i][pr]){
							which_guy.push_back(pr);
							which_guy.push_back(i);
							which_strategy.push_back(pr);
							which_strategy.push_back(j);
						}
					}
				}
			}

			if(learning) {
				diff = paper_nash;
				// go trough used_profile which has not changed!
				for (int i = 0; i < size; i++)
					for (int j = i + 1; j < size; j++){
						// subtract all costs of the proposed profile from Nash
						if (used_profile[pr][i][j] > 0) {
							diff -= learn_costs[edge(i,j)];
						}
					}
				if ( diff < 0) 
				{
					// Nash is smaller than proposed edges of profile
					//cout << pr <<"-th strategy profile is sligtly heavier than Nash and the difference is "<< diff << endl;
					heavy_profile[pr] = 1;
				}
			}
		}
		cout << "We found " << which_guy.size()/2 << " changes we want to have a closer look on" <<endl;
/*		for(int i = 0; i<which_strategy.size(); i+=2){
			cout << which_strategy[i] << " " << which_strategy[i+1] << endl;
		}*/

/*			for(int j = 0; j < 60; j++){
				heavy_profile[ignor[j]] = 1;
			}*/

		// 6.2
		// Add constraints and solve each time lp. Do this recurivly and use data from which we learnd so far:

		//cout<<"heavy profile at nr 19: "<<heavy_profile[19]<<endl;
		cout<<"Start recursion\n";
		rec(0, profileOrder, n_variables, size, mult_paths, model, v_edges, used_profile, which_guy, which_strategy, heavy_profile, maximum, paths, profile_path, one, eps, alternative_paths, learning, profile_path_memory);
		cout<<"End recursion, rec="<<rec_count<<"\n";
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