#!/usr/bin/env python
#coding=utf8

from PoS_solvers import *
from graph_tools import *
import networkx as nx

G, terminals = readGraphAndTerminals()
usedEdges, playerEdges, cost = findOptimum(G, terminals)

def toString(edgeList):
  return ', '.join(["({0}, {1})".format(u,v) for u,v in edgeList])


print "-"*79
print "optimum cost:", cost
print "used edges:", toString([(u,v) for i,(u,v) in enumerate(G.edges()) if usedEdges[i]])
#for k in range(len(terminals)):
#  print "player", k+1, "uses:", toString([(u,v) for i,(u,v) in enumerate(G.edges()) if playerEdges[k][i]])