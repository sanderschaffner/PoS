#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <set>
#include <map>
#include <ctime>

#include <CGAL/basic.h>
#include <CGAL/QP_models.h>
#include <CGAL/QP_functions.h>

// choose exact integral type
#ifdef CGAL_USE_GMP
#include <CGAL/Gmpz.h>
typedef CGAL::Gmpz ET;
#else
#include <CGAL/MP_Float.h>
typedef CGAL::MP_Float ET;
#endif

// program and solution types
typedef CGAL::Quadratic_program<int> Program;
typedef CGAL::Quadratic_program_solution<ET> Solution;

using namespace std;

int edge(int i, int j){
	int tmp;
	if (i>j){
		tmp = j; 
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

int zero = -1;
int i,j,k, p[3] = {5,5,5};
int i0,i1,i2;
int profile = 0;
int used_profile[125][6][6];
vector <int> which_guy[125], which_strategy[125];
int kakana[125];
int pp[3][125];
int used[6][6];
int a[6][6], users[6][6];
vector <vector <int> >  paths[3];
vector <int> cur;
double ans = 0;
int one = 6000;

Program lp (CGAL::SMALLER, true, 0, false, 0);
Solution lp_s;
int d = 0;

bool is_special(int pr){
	//if (pr==1 || pr==2 || pr==3 || pr==4 || pr==31 || pr==6) return true; else 
	return false;

}

void rec(int pr, int constraint){
	if (pr==125) {d=0;cout<<"gagaia simon amas rato ar beWdav?  "<<lp_s<<"\n";return;}
	//profile pr is not a Nash equilibrium
	//one of the guys wants to change the strategy
	//cout<<pr<<"\n";
	int i,j,k;
	memset(used,0,sizeof(used));
	int coef[8];
	if (is_special(pr)){
		for (i=0;i<3;i++){
			//i-th guy wants to deviate
			for (j=0;j<5;j++){
				//to the j-th strategy
				if (j!=pp[i][pr]){
					for (int ii=0;ii<6;ii++)
						for (int jj=0;jj<6;jj++)
							used[ii][jj] = used_profile[pr][ii][jj];
					int t = pp[i][pr];
					memset(coef, 0, sizeof(coef));
					//now subtract from used pp[i][pr] strategy and insert j-th strategy instead
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
					for (k=0;k<8;k++) {lp.set_a(k, constraint, coef[k]);/*cout<<coef[k]<<"   ";*/}
					//cout<<"\n";
					lp.set_b(constraint, zero);
					//cout<<pr<<"   "<<constraint<<" \n\n";
					lp_s = CGAL::solve_quadratic_program(lp, ET());
					//cout<<"kakadui\n";
					//cout<<pr<<"    "<<-lp_s.objective_value()<<"\n";
					//cout<<pp[0][pr]<<"   "<<pp[1][pr]<<"   "<<pp[2][pr]<<"\n";
					//cout<<"\n\n\n";
					if (d) return;
					if (-lp_s.objective_value()>1.58)
						rec(pr+1, constraint+1);
					if (d) return;
					for (k=0;k<8;k++) lp.set_a(k, constraint, 0);
					lp.set_b(constraint, 0);				
				}
			}
		}
		if (d) return;
		//cout<<"vax Cemi aqamdec moxvedi?\n";
		//if nothing works out, then try profile inequality
		memset(coef, 0, sizeof(coef));
		coef[edge(0,3)] = coef[edge(1,4)] = coef[edge(2,5)] = 1;
		for (i=0;i<6;i++)
			for (j=i+1;j<6;j++){
				if (used_profile[pr][i][j]>0) coef[edge(i,j)]-=1;
			}
		for (k=0;k<8;k++) lp.set_a(k, constraint, coef[k]);
		lp.set_b(constraint, 0);
		lp_s = CGAL::solve_quadratic_program(lp, ET());
		//cout<<pr<<"\n\n";
		//cout<<lp_s<<"\n";
		//cout<<-lp_s.objective_value()<<"\n";
		//cout<<pp[0][pr]<<"   "<<pp[1][pr]<<"   "<<pp[2][pr]<<"\n";
		//cout<<"\n\n\n";
		if (d) return;
		if (-lp_s.objective_value()>1.58)
			rec(pr+1, constraint+1);
		if (d) return;
		for (k=0;k<8;k++) lp.set_a(k, constraint, 0);
		lp.set_b(constraint,0);
		return;
	} else{
		int rr = rand()%2;
		if (kakana[pr] && rr){
			memset(coef, 0, sizeof(coef));
			coef[edge(0,3)] = coef[edge(1,4)] = coef[edge(2,5)] = 1;
			for (i=0;i<6;i++)
				for (j=i+1;j<6;j++){
					if (used_profile[pr][i][j]>0) coef[edge(i,j)]-=1;
				}
			for (k=0;k<8;k++) lp.set_a(k, constraint, coef[k]);
			lp.set_b(constraint, 0);
			lp_s = CGAL::solve_quadratic_program(lp, ET());
			//cout<<pr<<"\n\n";
			//cout<<lp_s<<"\n";
			cout<<-lp_s.objective_value()<<"\n";
			//cout<<pp[0][pr]<<"   "<<pp[1][pr]<<"   "<<pp[2][pr]<<"\n";
			//cout<<"\n\n\n";
			if (d) return;
			if (-lp_s.objective_value()>1.574)
				rec(pr+1, constraint+1);
			if (d) return;
			for (k=0;k<8;k++) lp.set_a(k, constraint, 0);
			lp.set_b(constraint,0);
			return;
		}
		int tt = rand()%which_guy[pr].size();		
		//for (int tt=0;tt<which_guy[pr].size();tt++){
			i = which_guy[pr][tt];
			j = which_strategy[pr][tt];
			//cout<<pr<<"  "<<i<<"  "<<j<<"\n"; 
			for (int ii=0;ii<6;ii++)
				for (int jj=0;jj<6;jj++)
					used[ii][jj] = used_profile[pr][ii][jj];
			int t = pp[i][pr];
			memset(coef, 0, sizeof(coef));
			//now subtract from used pp[i][pr] strategy and insert j-th strategy instead
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
			for (k=0;k<8;k++) {lp.set_a(k, constraint, coef[k]);/*cout<<coef[k]<<"   ";*/}
			//cout<<"\n";
			lp.set_b(constraint, zero);
			//cout<<pr<<"   "<<constraint<<" \n\n";
			lp_s = CGAL::solve_quadratic_program(lp, ET());
			//cout<<"kakadui\n";
			cout<<pr<<"    "<<-lp_s.objective_value()<<"\n";
			//cout<<pp[0][pr]<<"   "<<pp[1][pr]<<"   "<<pp[2][pr]<<"\n";
			//cout<<"\n\n\n";
			if (d) return;
			if (-lp_s.objective_value()>1.574)
				rec(pr+1, constraint+1);
			if (d) return;
			for (k=0;k<8;k++) lp.set_a(k, constraint, 0);
			lp.set_b(constraint, 0);
		//}
	}
}

double arr[8] = {113,277,418,318,0,549,556,664};

int main(){
	//freopen("output.txt","w",stdout);
	//paths are created here
	cur.push_back(0);cur.push_back(3);
	paths[0].push_back(cur);cur.clear();
	cur.push_back(0);cur.push_back(1);cur.push_back(2);cur.push_back(4);cur.push_back(3);
	paths[0].push_back(cur);cur.clear();
	cur.push_back(0);cur.push_back(1);cur.push_back(4);cur.push_back(3);
	paths[0].push_back(cur);cur.clear();
	cur.push_back(0);cur.push_back(1);cur.push_back(2);cur.push_back(5);cur.push_back(3);
	paths[0].push_back(cur);cur.clear();
	cur.push_back(0);cur.push_back(1);cur.push_back(4);cur.push_back(2);cur.push_back(5);cur.push_back(3);
	paths[0].push_back(cur);cur.clear();
	

	cur.push_back(1);cur.push_back(4);
	paths[1].push_back(cur);cur.clear();
	cur.push_back(1);cur.push_back(2);cur.push_back(4);
	paths[1].push_back(cur);cur.clear();
	cur.push_back(1);cur.push_back(0);cur.push_back(3);cur.push_back(4);
	paths[1].push_back(cur);cur.clear();
	cur.push_back(1);cur.push_back(2);cur.push_back(5);cur.push_back(3);cur.push_back(4);
	paths[1].push_back(cur);cur.clear();
	cur.push_back(1);cur.push_back(0);cur.push_back(3);cur.push_back(5);cur.push_back(2);cur.push_back(4);
	paths[1].push_back(cur);cur.clear();

	cur.push_back(2);cur.push_back(5);
	paths[2].push_back(cur);cur.clear();
	cur.push_back(2);cur.push_back(4);cur.push_back(3);cur.push_back(5);
	paths[2].push_back(cur);cur.clear();
	cur.push_back(2);cur.push_back(1);cur.push_back(4);cur.push_back(3);cur.push_back(5);
	paths[2].push_back(cur);cur.clear();
	cur.push_back(2);cur.push_back(1);cur.push_back(0);cur.push_back(3);cur.push_back(5);
	paths[2].push_back(cur);cur.clear();
	cur.push_back(2);cur.push_back(4);cur.push_back(1);cur.push_back(0);cur.push_back(3);cur.push_back(5);
	paths[2].push_back(cur);cur.clear();


	//inequalities saying that the minimum spanning tree is the line
	int constraint = 0 ;
	lp.set_a(edge(1,2), constraint, 1);lp.set_a(edge(1,4), constraint, -1);lp.set_b(constraint, 0);constraint++;
	lp.set_a(edge(2,4), constraint, 1);lp.set_a(edge(1,4), constraint, -1);lp.set_b(constraint, 0);constraint++;
	lp.set_a(edge(0,1), constraint, 1);lp.set_a(edge(0,3), constraint, -1);lp.set_b(constraint, 0);constraint++;
	lp.set_a(edge(1,2), constraint, 1);lp.set_a(edge(0,3), constraint, -1);lp.set_b(constraint, 0);constraint++;
	lp.set_a(edge(2,4), constraint, 1);lp.set_a(edge(0,3), constraint, -1);lp.set_b(constraint, 0);constraint++;
	lp.set_a(edge(3,4), constraint, 1);lp.set_a(edge(0,3), constraint, -1);lp.set_b(constraint, 0);constraint++;
	lp.set_a(edge(2,4), constraint, 1);lp.set_a(edge(2,5), constraint, -1);lp.set_b(constraint, 0);constraint++;
	lp.set_a(edge(3,4), constraint, 1);lp.set_a(edge(2,5), constraint, -1);lp.set_b(constraint, 0);constraint++;
	lp.set_a(edge(3,5), constraint, 1);lp.set_a(edge(2,5), constraint, -1);lp.set_b(constraint, 0);constraint++;

	//cost of the spanning tree is 1
	lp.set_a(edge(0,1),constraint,1);lp.set_a(edge(1,2),constraint,1);lp.set_a(edge(2,4),constraint,1);lp.set_a(edge(3,4),constraint,1);lp.set_a(edge(3,5),constraint,1);lp.set_b(constraint,1);
	constraint++;
	//inequalities checking that the edges (0,3) (1,4) and (2,5) give a Nash equilibrium
	int used[6][6];
	memset(used, 0, sizeof(used));
	used[0][3] = used[3][0] = used[1][4] = used[4][1] = used[2][5] = used[5][2] = 1;
	for (i=0;i<3;i++){
		for (j=1;j<5;j++){
			lp.set_a(edge(i,i+3),constraint,one);
			cout<<edge(i,i+3)<<"         ";
			for (k=0;k<paths[i][j].size()-1;k++){
				//edge we consider is paths[j][k],paths[j][k+1]
				lp.set_a(edge(paths[i][j][k],paths[i][j][k+1]), constraint, -one/(used[paths[i][j][k]][paths[i][j][k+1]]+1));
				cout<<edge(paths[i][j][k],paths[i][j][k+1])<<"  "<<-one/(used[paths[i][j][k]][paths[i][j][k+1]]+1)<<"  ";
			}		
			lp.set_b(constraint,zero);
			cout<<"\n";
			constraint++;
		}
	}
	lp.set_c(edge(0,3),-1);lp.set_c(edge(1,4),-1);lp.set_c(edge(2,5),-1);
	for (i0=0;i0<p[0];i0++){
		for (i1=0;i1<p[1];i1++){
			for (i2=0;i2<p[2];i2++){
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
				if (i0==1 && i1==1 && i2==1){
					cout<<profile<<"\n";
					for (int ii=0;ii<6;ii++){
						for (int jj=0;jj<6;jj++)
							cout<<used_profile[profile][ii][jj]<<" ";
						cout<<"\n";
					}
				}	
				pp[0][profile] = i0;
				pp[1][profile] = i1;
				pp[2][profile] = i2;
				if (used_profile[profile][0][3]>0 && used_profile[profile][1][4]>0 && used_profile[profile][2][5]>0) cout<<profile<<"\n";
				profile++;
			}
		}
	}	
	/*lp_s = CGAL::solve_quadratic_program(lp, ET());
	cout<<"es ise mainc vicodeT\n";
	cout<<lp_s<<"\n";
	double cur = 0;
	int pr;
	for (pr=1;pr<125;pr++){
		d=0;
		for (i=0;i<3;i++){
			//i-th guy wants to deviate
			for (j=0;j<5;j++){
				//to the j-th strategy
				if (j!=pp[i][pr]){
					for (int ii=0;ii<6;ii++)
						for (int jj=0;jj<6;jj++)
							used[ii][jj] = used_profile[pr][ii][jj];
					int t = pp[i][pr];
					cur = 0;
					//now subtract from used pp[i][pr] strategy and insert j-th strategy instead
					for (k=0;k<paths[i][t].size()-1;k++){
						cur-=arr[edge(paths[i][t][k],paths[i][t][k+1])]/used[paths[i][t][k]][paths[i][t][k+1]];
						used[paths[i][t][k]][paths[i][t][k+1]]--;
						used[paths[i][t][k+1]][paths[i][t][k]]--;
					}
					if (pr==31){
						cout<<cur<<"\n";
					}
					//and now insert the new 
					for (k=0;k<paths[i][j].size()-1;k++){
						used[paths[i][j][k]][paths[i][j][k+1]]++;
						used[paths[i][j][k+1]][paths[i][j][k]]++;
						cur+=arr[edge(paths[i][j][k],paths[i][j][k+1])]/used[paths[i][j][k]][paths[i][j][k+1]];					
					}
					if (cur<0){
						if (pr>0)cout<<pr<<"  "<<i<<" guy wants to change to "<<j<<"-th strategy  "<<"and cur is "<<cur<<"\n  ";
						which_guy[pr].push_back(i);
						which_strategy[pr].push_back(j);
						d = 1;
					}					
				}
			}
		}
		//if (d) continue;
		cur = 0;
		cur+=arr[edge(0,3)] + arr[edge(1,4)] + arr[edge(2,5)];
		for (i=0;i<6;i++)
			for (j=i+1;j<6;j++){
				if (used_profile[pr][i][j]>0) cur-=arr[edge(i,j)];
			}
		if (cur<0) {cout<<pr<<"  the strategy profile is sligtly heavier and the difference is "<<cur<<"\n";d = 1;kakana[pr] = 1;} else {cout<<"\n";}
		if (d==0){cout<<"ui ui ui  "<<pr<<"\n";}	
		cout<<"\n";
	}

	srand(time(NULL));
	cout<<"vaa\n";
	d = 0;
	rec(1,constraint);
	cout<<"sgsgsg\n";*/
	
	return 0;
}
