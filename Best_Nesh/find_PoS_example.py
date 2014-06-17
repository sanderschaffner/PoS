#!/usr/bin/env python
#coding=utf8

from PoS_solvers import *
from graph_tools import *
import networkx as nx
from sys import stdin

print "How many vertices should the example have?"
n = int(stdin.readline())

print "Please give a list of pairs of terminals (in one line):"
terminals = [[int(x),int(y)] for x,y in readTuples(2)]

print "What is the highest PoS you know an example for?"
PoS = float(stdin.readline())

print "How many edges does your example have (I'll also try to find a smaller one)?"
bestEdges = int(stdin.readline())

print "How many iterations should I try?"
iterations = int(stdin.readline())

def isPromising(costOpt, costNE, nEdges):
  newPoS = costNE/float(costOpt)
  return newPoS > PoS or (newPoS == PoS and nEdges < bestEdges)

for i in range(iterations):
  if i%(iterations/10) == 0:
    print 100*i/iterations, "%"
  
  G = randomGraph(n)
  sol,pEdges,OPT = findOptimum(G, terminals)
  
  # try first heuristic
  nashEq,NE = findNashEqFromConfig(G, terminals, pEdges)
  if(not isPromising(OPT, NE, G.number_of_edges())):
    continue
  
  # try second heuristic
  nashEq,NE = findBetterNashEq(G, terminals)
  if(not isPromising(OPT, NE, G.number_of_edges())):
    continue
  
  # solving exactly...
  nashEq,NE = findBestNashEq(G, terminals)
  if(not isPromising(OPT, NE, G.number_of_edges())):
    continue
    
  PoS = NE/float(OPT)
  bestEdges = G.number_of_edges()
  print "BETTER EXAMPLE FOUND!"
  print "G =", ", ".join([str((('(%d, %d, %.4f)' % (u, v, cost,)).rstrip('0').rstrip('.'))) for (u,v),cost in nx.get_edge_attributes(G, 'cost').items()])
  print "OPT =", OPT, ": ", map(int, sol)
  print "NE =", NE, ": ", map(int, nashEq)
  print "PoS =", round(PoS, 5), ", nEdges =", bestEdges
  print "-"*79
  
  