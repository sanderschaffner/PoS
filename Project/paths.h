/*
 * permutation functions
 */
#include <stdio.h> // permutation
#include <vector>

class Paths
{
private:
	std::vector< std::vector<int> > paths_1; // possible paths without permutations and without source and target
	std::vector< std::vector<int> > paths_2; // possible paths with permutations and without source and target
	std::vector< std::vector<int> > paths; // existing paths
	std::vector<int> tmp;
	std::vector<int> tmp2;
	std::vector<int> elements;
	unsigned int source;
	unsigned int target;
	unsigned int nElements;
	bool temp;

	// Takes a set of numbers, eg {1,2,3} and saves all permutations of it into paths
	// http://www.bearcave.com/random_hacks/permute.html
	void permute(int *v, const int start, const int n)
	{  
		if (start == n-1) {
			//print(v, n);
			tmp.clear();
			for (int i = 0; i < n; i++) {
				tmp.push_back(v[i]);
			}
			paths_2.push_back(tmp);
		}
		else {
			for (int i = start; i < n; i++) {
				int tmp = v[i];

				v[i] = v[start];
				v[start] = tmp;
				permute(v, start+1, n);
				v[start] = v[i];
				v[i] = tmp;
			}
		}
	}

	// recursivly write one path in each step into paths_1
	void recGetOnePath(std::vector<int> previous, int start)
	{
		for(int i = start+1; i< elements.size(); i++)
		{
			tmp = previous;
			tmp.push_back(elements[i]);
			paths_1.push_back(tmp);

			if(i < elements.size())
			{
				recGetOnePath(tmp, i);
			}
		}
	}

public:
	Paths(unsigned int s, unsigned int t, unsigned int nE)
	{
		source = s;
		target = t;
		nElements = nE;

		// need all other elements which differ from s and t
		for(int i = 0; i < nElements; i++){
			if(i != s && i != t) {
				elements.push_back(i);
			}
		}
	}

	std::vector< std::vector<int> > getExistingPaths()
	{
		return paths;
	}

	void getAllPaths(vector< vector< unsigned int > > graph)
	{
		// take all possible nodes inbetween s and t. Then permutate them to get all possible paths.
		// after that reduce number of nodes by one node and repeat s.t. we have also all possible number of shorter paths from s to t
		for(int i = 0; i < elements.size(); i++) {
			tmp.clear();
			tmp.push_back(elements[i]);
			paths_1.push_back(tmp);
			recGetOnePath(tmp, i);
		}

		// get the permutations of the paths before
		for(int i = 0; i < paths_1.size(); i++)
		{
			// write vector into array and permutate
			tmp = paths_1[i];
			int v[tmp.size()];
			for(int j = 0; j < tmp.size(); j++)
			{
				v[j] = tmp[j];
			}
			permute(v, 0, sizeof(v)/sizeof(int));
		}

		// print paths:
/*		for (int i = 0; i < paths_2.size(); i++) {
			tmp = paths_2[i];
			for(int j = 0; j < tmp.size(); j++) {
				std::cout << tmp[j];
			}
			printf("\n");
	    }*/

		// go trough all paths and ensure that it is valid according to the graph:
		// a: direct path: source->target
		if(graph[source][target])
		{
			tmp2.clear();
			tmp2.push_back(source);
			tmp2.push_back(target);
			paths.push_back(tmp2);
		}
		// b: indirect paths: source->xxx->target
		tmp.clear();
		for(int i = 0; i < paths_2.size(); i++)
		{
			tmp = paths_2[i];
			temp = true;
			for(int i = 0; i < tmp.size()+1; i++)
			{
				if(i == 0) {
					if( !graph[source][tmp[i]] ) {
						temp = false;
						//std::cout << "anfang" << std::endl;
					}					
				} else if(i == tmp.size()) {
					if( !graph[tmp[i-1]][target] ) {
						temp = false;
						//std::cout << "ende" << std::endl;
					}
				} else {
					if( !graph[tmp[i-1]][tmp[i]] ) {
						temp = false;
						//std::cout << "dazwischen" << std::endl;
					}
				}
			}
		
			//std::cout << tmp.size() << std::endl;

			if( temp )
			{
				tmp2.clear();
				tmp2.push_back(source);
				std::vector<int>::iterator it;

				it = tmp2.begin();
				tmp2.insert (it+1,tmp.begin(),tmp.end());
				tmp2.push_back(target);
				paths.push_back(tmp2);
			}

			//std::cout << "source/target " << source << " " << target << " " << paths.size() << std::endl;
		}
	}

	void print()
	{
	    for (int i = 0; i < paths.size(); i++) {
			tmp = paths[i];
			for(int j = 0; j < tmp.size(); j++) {
				std::cout << tmp[j];
			}
			printf("\n");
	    }
	}
};