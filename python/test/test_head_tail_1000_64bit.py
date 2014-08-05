#!/usr/bin/python
import bvg
import sys

data = sys.argv[1]
G = bvg.BVGraph(data, 0) # load with offset step = 0 (iteration)
print '#nodes = ' + str(G.nverts) + ', #edges = ' + str(G.nedges)
edges_and_degrees = G.edges_and_degrees()
const = 10000000001; count = 0
for (src, dst, degree) in edges_and_degrees:
    count += 1
    if degree == 1 and dst + src != const:
        print 'error: reading 64bit bvgraph, src = ' + str(src) + ', dst = ' + str(dst) 
        break
    elif count == 1000:
        print 'Python: testing 64-bit bvgraph sequential read ... passed!'
        break
