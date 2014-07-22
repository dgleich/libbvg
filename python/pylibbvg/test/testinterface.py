#!/usr/bin/python
import bvg
import sys

## new an object without loading graph
#G = bvg.Graph()

## new an object with graph loaded
data = sys.argv[1]
G1 = bvg.BVGraph(data, 0)  # load with offset step = 0 (iteration)
G2 = bvg.BVGraph(data, 1)  # load with offset step = 1 (random access)

node = int(sys.argv[2])  # node is given

## test successors and degree
a = G2.successors(node)
for v in a:
    print v
print 'Degree = ' + str(G2.out_degree(node))

## test direct access to number of nodes and edges
print G1.nverts
print G1.nedges

## test iteration in G
for v in G1:
    print v
#    src = v.curr
#    deg = v.degree
#    print 'src = ' + str(src) + ', degree = ' + str(deg)
#    for neigh in v:
#        dst = neigh
#        print '  ' + str(src) + ' --- ' + str(dst)

## test random access for vertex
v = G2.vertex(node)
print v.curr
print v.degree

#for neigh in v:
#    dst = neigh
#    print '  ' + str(v.curr) + ' --- ' + str(dst)

## test node iterator
#for n in G1.nodes():
#    print n

## test edge iterator
#edges = G1.edges()
#for e in edges:
#    print str(e[0]) + ' --- ' + str(e[1])

#for (src, dst) in edges:
#    print str(src) + ' --- ' + str(dst)

## test edge-degree iterator
#edges_and_degrees = G1.edges_and_degrees()
#for e in edges_and_degrees:
#    print str(e[0]) + ' --- ' + str(e[1]) + ' [' + str(e[2]) + ']'
#for (src, dst, degree) in edges_and_degrees:
#    print str(src) + ' --- ' + str(dst) + ' [' + str(degree) + ']'

## test vertex iterator
#vertices = G1.vertices()
#for v in vertices:
#    src = v.curr
#    deg = v.degree
#    print 'src = ' + str(src) + ', degree = ' + str(deg)
#    for neigh in v:
#        dst = neigh
#        print '  ' + str(src) + ' --- ' + str(dst)

## test length
#print len(G1)

## test in operator
#if node in G1:
#    print 'node in Graph'
#else:
#    print 'node not in Graph'

## test load_graph
#G = bvg.load_graph(data, offsets=True)
#if node in G:
#    print 'node in new G'

## test random iterator
#nodes = [1, 2, 10, 20, 100, 200, 300]
#R1 = G2.random_iterator(nodes)

#for (node, successors) in R1:
#    print node
#    for i in range(0, len(successors)):
#        print '  ' + str(node) + ' --- ' + str(i)

## test adjacency_iter()
#for n, v in G1.adjacency_iter():
#    print 'node ' + str(n) + ' has neighbors:'
#    for nbr, attr in v.items():
#        print '  ' + str(nbr) + ' with weight ' + str(attr)

