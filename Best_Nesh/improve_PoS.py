from PoS_solvers import *
from graph_tools import *
import networkx as nx

G, terminals = readGraphAndTerminals()

def getPoS():
  sol,pEdges,OPT = findOptimum(G, terminals)
  nashEq,NE = findBestNashEq(G, terminals)
  return NE/OPT

def improveEdgeCost(u,v,currPoS):
  epsilon = 0.000000001
  originalCost = G[u][v]['cost']
  
  fak = 0
  G[u][v]['cost'] = originalCost + epsilon
  PoS1 = getPoS()
  PoS = currPoS
  if(PoS1 > PoS):
    PoS = PoS1
    fak = 1
  G[u][v]['cost'] = originalCost - epsilon
  PoS2 = getPoS()
  if(PoS2 > PoS):
    PoS = PoS2
    fak = -1
  
  if(fak == 0):
    G[u][v]['cost'] = originalCost
    return currPoS
  
  epsilon *= fak
  
  oldPoS = currPoS
  while(PoS > oldPoS):
    epsilon *= 2
    G[u][v]['cost'] = originalCost + epsilon
    oldPoS = PoS
    PoS = getPoS()
  
  def binarySearch(loEpsilon, hiEpsilon, loPoS, hiPoS):
    if(loPoS < hiPoS + 0.00001):
      G[u][v]['cost'] = originalCost + loEpsilon
      return loPoS
    midEpsilon = (loEpsilon + hiEpsilon)/2
    G[u][v]['cost'] = originalCost + midEpsilon
    midPoS = getPoS()
    
    if(midPoS > loPoS):
      return binarySearch(midEpsilon, hiEpsilon, midPoS, hiPoS)
    else:
      return binarySearch(loEpsilon, midEpsilon, loPoS, midPoS)
  
  return binarySearch(epsilon/2, epsilon, oldPoS, PoS)

oldPoS = getPoS()
newPoS = oldPoS

for i,(u,v) in enumerate(G.edges()):
  print "edge", i+1, "/", G.number_of_edges()
  newPoS = improveEdgeCost(u,v,newPoS)

sol,pEdges,OPT = findOptimum(G, terminals)
nashEq,NE = findBestNashEq(G, terminals)

print "G =", ", ".join([str((('(%d, %d, %.9f)' % (u, v, cost,)).rstrip('0').rstrip('.'))) for (u,v),cost in nx.get_edge_attributes(G, 'cost').items()])
print "OPT =", OPT, ": ", map(int, sol)
print "NE =", NE, ": ", map(int, nashEq)
print "PoS: ", oldPoS, "->", NE/OPT
