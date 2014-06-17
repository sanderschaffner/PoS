#coding=utf8

from re import compile,sub
import networkx as nx
import sys
from random import randint
from random import random

def readTuples(k):
  values = []
  while(len(values)%k != 0 or values == []):
    delimiters = compile('[^0-9.]+')
    ends = compile('^[^0-9]*|[^0-9]*$')
    
    values = delimiters.sub(' ', ends.sub('', sys.stdin.readline())).split(' ')
    
    if(len(values)%k != 0):
      print "number of values not divisible by", k, ":-)"
  return [values[i:i+k] for i in range(0, len(values), k)]


def readGraphAndTerminals():
  print("enter edges (e.g. '[0,1,cost1],[3,4,cost2],...' with arbitrary delimiters):")
  G = nx.Graph()
  for x,y,c in readTuples(3):
    G.add_edge(int(x),int(y),cost=float(c))
  
  print("enter terminals (e.g. '[0,1],[1,2],...' or '0 1 1 2 ...', etc):")
  
  return G, [[int(x),int(y)] for x,y in readTuples(2)]


def drawSolution(G, sol, nashEq, terminals=[], sharedNode=None):
  pos = nx.spring_layout(G) 
  #nx.draw(G, pos)
  nx.draw_networkx_edges(G, pos)
  edges = [e for i,e in enumerate(G.edges()) if sol[i]]
  nx.draw_networkx_edges(G, pos, edgelist=edges, width=3)
  edges = [e for i,e in enumerate(G.edges()) if nashEq[i]]
  nx.draw_networkx_edges(G, pos, edgelist=edges, width=2, style='dashed', edge_color='R')
  edge_labels = dict([((u,v),d['cost']) for u,v,d in G.edges(data=True)]) 
  nx.draw_networkx_edge_labels(G,pos,edge_labels=edge_labels)
  
  nodeset = [v for u,v in terminals].union([v for v,u in terminals])
  nx.draw_networkx_nodes(G,pos,nodelist=[v for v in G.nodes() if v != sharedNode and v not in nodeset],node_color='w',node_size=500)
  
  for i,(u,v) in enumerate(terminals):
    for x in (u,v):
      if x != sharedNode:
        nx.draw_networkx_nodes(G,pos,nodelist=[x],node_color='r',node_shape='s^>v<dph8'[i%len(terminals)],node_size=700)
  if sharedNode != None:
    nx.draw_networkx_nodes(G,pos,nodelist=[sharedNode],node_color='r',node_shape='o', linewidths=2,node_size=600)  
  nx.draw_networkx_labels(G,pos)
  plt.axis('off',axisbg='w')


def drawExample(G, terminals):
  usedEdges, _, cost = findOptimum(G, terminals)
  bestNE, cost = findBestNashEq(G, terminals)
  
  shared = None
  for v in terminals[0]:
    vShared = True
    for t in terminals[1:]:
      if v not in t:
        vShared = False
    if vShared:
      shared = v
  drawSolution(G, usedEdges, bestNE, terminals, shared)


def randomGraph(n):
  active = True;
  while(active):
    p = random()
    for i in range(100):
      G = nx.gnp_random_graph(n,p)
      if(nx.is_connected(G)):
        active = False
        break
  for (u,v) in G.edges():
    G[u][v]['cost'] = randint(1,n) + randint(0,n)/10000.
  return G
