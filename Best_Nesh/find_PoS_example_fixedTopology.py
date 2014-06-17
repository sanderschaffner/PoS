#!/usr/bin/env python
#coding=utf8

from PoS_solvers import *
from graph_tools import *
import networkx as nx
from sys import stdin

from random import random

G, terminals = readUnweightedGraphAndTerminals()

print "What is the highest PoS you know an example for?"
PoS = float(stdin.readline())

bestEdges = 0

def randomizeCosts():
  for (u,v) in G.edges():
    G[u][v]['cost'] = random()

def isPromising(costOpt, costNE, nEdges):
  newPoS = costNE/float(costOpt)
  return newPoS > PoS or (newPoS == PoS and nEdges < bestEdges)

while(True):
  randomizeCosts()
  
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
  print "G =", ", ".join([str((('(%d, %d, %.9f)' % (u, v, cost,)).rstrip('0').rstrip('.'))) for (u,v),cost in nx.get_edge_attributes(G, 'cost').items()])
  print "OPT =", OPT, ": ", map(int, sol)
  print "NE =", NE, ": ", map(int, nashEq)
  print "PoS =", round(PoS, 5), ", nEdges =", bestEdges
  print "-"*79