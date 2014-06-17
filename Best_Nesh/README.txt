Yann:
This directory contains my optimization scripts that I feel are worthy to be used by others. In order to execute them, you will need a functioning python2.6 installation at well as the 'networkx' and 'gurobipy' modules. 

At the moment the following scripts are available:

- find_ne.py: finds the best nash equilibrium for a given graph
- find_opt.py: finds the optimum solution for a given graph
- find_PoS_example.py: tries random graphs in order to find an example with highest PoS
- find_PoS_example_fixedTopology.py: tries random edge costs in order to find an example with highest PoS with the given structure
- improve_PoS.py: takes an example and tries to improve it by modifying the costs slightly

The scripts should ask quite verbosely for what they want. Graphs are generally expected as a list of edges in a single line, i.e. triples of values (v1,v2,cost) that are surrounded by arbitrary delimiters (VERTICES ARE 0-BASED INDICES!). Similarly, a list of terminals is expected to consist of tuples on a single line. (Of course input can be piped in from a file)

the following files are neccessary functions for the above scripts:
- PoS_solvers.py
- graph_tools.py