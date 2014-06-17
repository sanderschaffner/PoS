#!/usr/bin/env python
#coding=utf8

import sys
from gurobipy import *
import networkx as nx
from itertools import permutations

''' finds the optimum solution for the connection game in graph G
    G: undirected input graph with 'cost' values on the edges.
    terminals: list of pairs of terminals. the length of the list equals k.
    
    returns: a triple (usedEdges, playerEdges, cost)
    usedEdges: a list of Bools indicating whether edge i is part of the solution
    playerEdges: a list of lists of bools. one list for each player and one bool 
                 for each edge that indicates whether this edge lies on the path
                 of the player in the solution
    cost: the cost of the solution
'''
def findOptimum(G, terminals):
  m = Model("opt_mip")
  
  useEdge = {}
  for u,v,cost in G.edges(data=True):
    useEdge[u,v] = m.addVar(vtype=GRB.BINARY, name=str(('useEdge',(u,v))), obj=cost['cost'])
  
  nPlayers = len(terminals)
  
  flow = {}
  for i,(u,v) in enumerate(G.edges()):
    for j in range(nPlayers):
      flow[j,u,v] = m.addVar(lb=-1, ub=1, vtype=GRB.INTEGER, name=str(('flow',j,(u,v))))
  
  m.update()
  
  for i,(u,v) in enumerate(G.edges()):
    for j in range(nPlayers):
      m.addConstr(flow[j,u,v] >= -1*useEdge[u,v])
      m.addConstr(flow[j,u,v] <= useEdge[u,v])
  
  for j in range(nPlayers):
    demand = [0]*G.number_of_nodes();
    demand[terminals[j][0]] = 1
    demand[terminals[j][1]] = -1
    
    nodeConstraints = []
    for l in range(G.number_of_nodes()):
      nodeConstraints.append(m.addConstr(lhs=0, sense=GRB.EQUAL, rhs=demand[l]))
    
    m.update()
    
    for i,(u,v) in enumerate(G.edges()):
      m.chgCoeff(nodeConstraints[u], flow[j,u,v], 1)
      m.chgCoeff(nodeConstraints[v], flow[j,u,v], -1)
      m.update()
  
  m.setParam('OutputFlag', False)
  m.optimize()
  
  usedEdges = [useEdge[(u,v)].x for (u,v) in G.edges()]
  playerEdges = [[flow[(p,u,v)].x != 0 for (u,v) in G.edges()] for p in range(nPlayers)]
  cost = m.objVal
  
  return usedEdges, playerEdges, cost


''' finds the cheapest Nash equilibrium for the connection game in graph G
    G: undirected input graph with 'cost' values on the edges.
    terminals: list of pairs of terminals. the length of the list equals k.
    
    returns: a tuple (usedEdges, cost)
    usedEdges: a list of Bools indicating whether edge i is part of the solution
    cost: the cost of the solution
'''
def findBestNashEq(G, terminals):
  m = Model("ne_mip")
  
  nPlayers = len(terminals)
  P,E,V = range(nPlayers),G.edges(),G.nodes()
  
  C = 0
  for u,v,cost in G.edges(data=True):
    C += cost['cost']
  
  # minimize Σ_e Σ_k c_e count_e^k
  count = {}
  for u,v,cost in G.edges(data=True):
    count[u,v,0] = m.addVar(vtype=GRB.BINARY, name=str(('count',u,v,0)))
    for k in range(1, nPlayers+1):
      count[u,v,k] = m.addVar(vtype=GRB.BINARY, name=str(('count',u,v,k)), obj=cost['cost'])
  
  # setup variables
  unusedEdge,usedEdge,edgeOrientation,usedArc,spTree,effCost,usedVertex,spTreeLvl,d = {},{},{},{},{},{},{},{},{}
  for p in P:
    for u,v in E:
      unusedEdge[p,u,v] = m.addVar(vtype=GRB.BINARY, name=str(('unusedEdge',p,u,v)))
      for k in range(nPlayers):
        usedEdge[p,u,v,k] = m.addVar(vtype=GRB.BINARY, name=str(('usedEdge',p,u,v,k)))
      for x,y in ((u,v),(v,u)):
        edgeOrientation[p,x,y] = m.addVar(vtype=GRB.BINARY, name=str(('edgeOrientation',p,x,y)))
        usedArc[p,x,y] = m.addVar(vtype=GRB.BINARY, name=str(('usedArc',p,x,y)))
        spTree[p,x,y] = m.addVar(vtype=GRB.BINARY, name=str(('spTree',p,x,y)))
        effCost[p,u,v] = m.addVar(lb=0, ub=C, vtype=GRB.CONTINUOUS, name=str(('effCost',p,u,v)))
    for v in V:
      usedVertex[p,v] = m.addVar(vtype=GRB.BINARY, name=str(('usedVertex',p,v)))
      spTreeLvl[p,v] = m.addVar(lb=0, ub=len(V), vtype=GRB.INTEGER, name=str(('spTreeLvl',p,v)))
      d[p,v] = m.addVar(lb=0, ub=C, vtype=GRB.CONTINUOUS, name=str(('d',p,v)))
  m.update()
  
  for p in P:
    m.addConstr(usedVertex[p,terminals[p][0]] == 1)
    m.addConstr(usedVertex[p,terminals[p][1]] == 1)
    m.addConstr(d[p,terminals[p][0]] == 0)
  
  # Σ_k count_e^k = 1
  for u,v in E:
    m.addConstr(sum(count[u,v,k] for k in range(nPlayers+1)) == 1)
  
  # unusedEdge_p,e + Σ_k usedEdge_p,e^k = 1
  for p in P:
    for u,v in E:
      m.addConstr(unusedEdge[p,u,v] + sum(usedEdge[p,u,v,k] for k in range(nPlayers)) == 1)
  
  # Σ_p usedEdge_p,e^k = (k+1) count_e^(k+1)
  for u,v in E:
    for k in range(nPlayers):
      m.addConstr(sum(usedEdge[p,u,v,k] for p in P) == (k+1)*count[u,v,k+1])
  
  # edgeOrientation_p,u,v + edgeOrientation_p,v,u = 1
  for p in P:
    for u,v in E:
      m.addConstr(edgeOrientation[p,u,v] + edgeOrientation[p,v,u] == 1)
  
  # 2*usedArc_p,u,v ≤ 1 + edgeOrientation_p,u,v - unusedEdge_p,{u,v}
  for p in P:
    for x,y in E:
      for u,v in ((x,y),(y,x)):
        m.addConstr(2*usedArc[p,u,v] <= 1 + edgeOrientation[p,u,v] - unusedEdge[p,x,y])
  
  # usedArc_p,u,v + usedArc_p,v,u + unusedEdge_p,{u,v} = 1
  for p in P:
    for u,v in E:
      m.addConstr(usedArc[p,u,v] + usedArc[p,v,u] + unusedEdge[p,u,v] == 1)
  
  # Σ_{u,fixed v} spTree_p,u,v = 1
  for p in P:
    for v in V:
      if v != terminals[p][0]:
        m.addConstr(sum(spTree[p,u,v] for _,u in G.edges(v)) == 1)
  
  # spTree_p,u,v ≤ edgeOrientation_p,u,v
  for p in P:
    for x,y in E:
      for u,v in ((x,y),(y,x)):
        m.addConstr(spTree[p,u,v] <= edgeOrientation[p,u,v])
  
  # spTreeLvl_p,v ≥ 0.5 + spTreeLvl_p,u - |V| + |V|*spTree_p,u,v
  for p in P:
    for x,y in E:
      for u,v in ((x,y),(y,x)):
        m.addConstr(spTreeLvl[p,v] >= 0.5 + spTreeLvl[p,u] - len(V) + len(V)*spTree[p,u,v])
  
  # Σ_{fixed u,v} usedArc_p,u,v = usedVertex_p,u
  for p in P:
    for u in V:
      if u != terminals[p][1]:
        m.addConstr(sum(usedArc[p,u,v] for _,v in G.edges(u)) == usedVertex[p,u])
      if u != terminals[p][0]:
        m.addConstr(sum(usedArc[p,v,u] for _,v in G.edges(u)) == usedVertex[p,u])
  
  # effCost_p,e = c_e*Σ_k 1/(k+1)*count_e^k + c_e*Σ_k (1/(k+1) - 1/(k+2))*usedEdge_p,e^k
  for p in P:
    for u,v,cost in G.edges(data=True):
      c = cost['cost']
      m.addConstr(effCost[p,u,v] == sum(c/(k+1.0)*count[u,v,k] for k in range(nPlayers+1)) + 
                                    sum(c*(1/(k+1.0)-1/(k+2.0))*usedEdge[p,u,v,k] for k in range(nPlayers)))
  
  # d_p,v ≤ d_p,u + effCost_p,{u,v}
  # d_p,v ≥ d_p,u + effCost_p,{u,v} - C + C*spTree_p,u,v
  for p in P:
    for x,y in E:
      for u,v in ((x,y),(y,x)):
        m.addConstr(d[p,v] <= d[p,u] + effCost[p,x,y])
        m.addConstr(d[p,v] >= d[p,u] + effCost[p,x,y] - C + C*spTree[p,u,v])
  
  # d_p,v ≥ d_p,u + c_{u,v}*Σ_k(1/(k+1)*usedEdge_p,{u,v}^k) - C + C*usedArc_p,u,v
  for p in P:
    for x,y,cost in G.edges(data=True):
      for u,v in ((x,y),(y,x)):
        m.addConstr(d[p,v] >= d[p,u] + effCost[p,x,y] - C + C*usedArc[p,u,v])
        m.addConstr(d[p,v] <= d[p,u] + effCost[p,x,y] + C - C*usedArc[p,u,v])
  
  m.setParam('OutputFlag', False)
  m.optimize()
  
  pEdges = [[1 - unusedEdge[p,u,v].x for (u,v) in G.edges()] for p in P]
  nashEq = [False]*len(E)
  for p in P:
    nashEq = [nashEq[j] or pEdges[p][j] for j in range(len(E))]
  cost = m.objVal
  '''
  print G.edges()
  print pEdges
  print nashEq
  '''
  return nashEq, cost


''' finds the Nash equilibrium that results from best-response dynamics starting
    with the given configuration
    G: undirected input graph with 'cost' values on the edges.
    terminals: list of pairs of terminals. the length of the list equals k.
    pEdges: a list of lists of bools. one list for each player and one bool 
            for each edge that indicates whether this edge currently lies on 
            the path of the player
    
    returns: a tuple (usedEdges, cost)
    usedEdges: a list of Bools indicating whether edge i is part of the solution
    cost: the cost of the solution
'''
def findNashEqFromConfig(G, terminals, pEdges):
  active = True
  
  m,p = G.number_of_edges(),len(terminals)
  
  while(active):
    active = False
    for i in range(p):
      hist = [0]*m
      for j in range(p):
        if(j == i):
          continue
        hist = [hist[k]+int(pEdges[j][k]) for k in range(m)]
      for j,(u,v) in enumerate(G.edges()):
        G[u][v]['weight'] = G[u][v]['cost']/float(hist[j]+1)
        
      sp = nx.shortest_path(G, terminals[i][0], terminals[i][1], True)
      edgeSet = [(sp[j],sp[j+1]) for j in range(len(sp)-1)]
      edges = [(u,v) in edgeSet or (v,u) in edgeSet for (u,v) in G.edges()]
      
      oldPrice, newPrice = 0, 0
      for j,(u,v,cost) in enumerate(G.edges(data=True)):
        if(pEdges[i][j]):
          oldPrice += cost['cost']/float(hist[j]+1)
        if(edges[j]):
          newPrice += cost['cost']/float(hist[j]+1)
          
      #if(edges != pEdges[i]):
      if(newPrice < oldPrice):
        active = True
        pEdges[i] = edges
        
  nashEq = [False]*m
  for i in range(p):
    nashEq = [nashEq[j] or pEdges[i][j] for j in range(m)]
  cost = sum([G[u][v]['cost'] for i,(u,v) in enumerate(G.edges()) if nashEq[i]])
  return nashEq, cost


''' finds the best Nash equilibrium that results from best-response dynamics 
    starting with configurations in which a player order is fixed and then each 
    player in order picks his currently cheapest path
    G: undirected input graph with 'cost' values on the edges.
    terminals: list of pairs of terminals. the length of the list equals k.
    pEdges: a list of lists of bools. one list for each player and one bool 
            for each edge that indicates whether this edge currently lies on 
            the path of the player
            
    returns: a tuple (usedEdges, cost)
    usedEdges: a list of Bools indicating whether edge i is part of the solution
    cost: the cost of the solution
'''
def findBetterNashEq(G, terminals):
  sol,playerEdges,c = findOptimum(G, terminals)
  bestEq, bestCost = findNashEqFromConfig(G, terminals, playerEdges)
  
  m,p = G.number_of_edges(),len(terminals)
  
  for playerOrder in permutations(range(p)):
    hist = [0]*m
    
    for player in playerOrder:
      for i,(u,v) in enumerate(G.edges()):
        G[u][v]['weight'] = G[u][v]['cost']/float(hist[i]+1)
        
      sp = nx.shortest_path(G, terminals[player][0], terminals[player][1], True)
      edgeSet = [(sp[j],sp[j+1]) for j in range(len(sp)-1)]
      playerEdges[player] = [(u,v) in edgeSet or (v,u) in edgeSet for (u,v) in G.edges()]
      hist = [hist[k]+int(playerEdges[player][k]) for k in range(m)]
      
    nashEq, cost = findNashEqFromConfig(G, terminals, playerEdges)
    if cost < bestCost:
      bestCost, bestEq = cost, nashEq
      
  return bestEq, bestCost


'''
G = nx.Graph()
G.add_edge(1,2,cost=3)
G.add_edge(1,5,cost=4)
G.add_edge(1,0,cost=4)
G.add_edge(1,3,cost=4)
G.add_edge(1,4,cost=4)
G.add_edge(2,5,cost=3)
G.add_edge(5,3,cost=2)
G.add_edge(0,3,cost=2)
G.add_edge(3,4,cost=2)
terminals = [[1,2],[1,5],[1,0],[1,3],[1,4]]
#print("\n".join(map(str,findOptimum(G, terminals))))
print("\n".join(map(str,findBestNashEq(G, terminals))))
'''