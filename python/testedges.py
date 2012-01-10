#!/usr/bin/env python
import libbvg as bvg
import networkx as nx

G = bvg.BVGraph('../../data/harvard500', 1)

print G.degree(1)

for src, dst, data in G.edges(data=True):
    print src, " -> ", dst, " ", data

