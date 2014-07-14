#!/usr/bin/python
import bvg
import sys

data = sys.argv[1]
G = bvg.BVGraph(data, 0) # load with offset step = 0 (iteration)
print '#nodes = ' + str(G.nverts) + ', #edges = ' + str(G.nedges)
edges_and_degrees = G.edges_and_degrees()
for (src, dst, degree) in edges_and_degrees:
    if degree == 1 and dst != src + 1:
        print 'error: reading 64bit bvgraph'
        break
    elif src > 1000:
        print 'Python: testing 64-bit bvgraph sequential read ... passed!'
        break