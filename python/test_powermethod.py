#!/usr/bin/env python

import libbvg as bvg
import sys

def matvec(G, x, alpha=0.85):
    """A Matrix-Vector product procedure.
    G is a BVGraph object.
    x is a vector (list).
    """
    for v in G.vertices():
        i = v.curr
        product = 0
        for neigh in v:
            j = neigh   # A[i][j] = 1
            product = product + alpha * x[j]

        x[i] = product + 1 - alpha

    return x

data = sys.argv[1]
G = bvg.BVGraph(data)
alpha = 0.85

x = [ 1 for i in xrange(0, G.nverts)]
r = [ 1 for i in xrange(0, G.nverts)]

for v in G.vertices():
    i = v.curr
    product = 0
    for neigh in v:
        j = neigh
        product = product + alpha * x[j]
    r[i] = r[i] - (1 - product)
    print r[i]

for k in range(1, 10):
    for v in G.vertices():
        i = v.curr
        x[i] = x[i] + r[i]
        product = 0
        for neigh in v:
            j = neigh
            product = product + alpha
        r[i] = r[i] + r[i] * (1-product)

        print r[i]

    s = raw_input()
