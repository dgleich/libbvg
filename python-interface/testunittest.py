#!/usr/bin/python
import libbvg as bvg
import unittest
import sys
import random

class BVGraphTest(unittest.TestCase):
    def __init__(self,filename):
        unittest.TestCase.__init__(self)
        self.graphname = sys.argv[1]
    def setUp(self):
        smatfile = open(self.graphname+'.smat','rt')
        header = smatfile.readline().split()
        self.check_nnodes = int(header[0])
        self.check_nedges = int(header[2])
        self.assertEqual(self.check_nnodes, int(header[1])) # make sure it's square
        self.checkgraph = {}
        for i in xrange(0, self.check_nnodes):
            self.checkgraph[i] = set()
        for line in smatfile:
            parts = line.split()
            src = int(parts[0])
            dst = int(parts[1])
            #if src not in self.checkgraph:
            #    self.checkgraph[src] = set()
            assert(dst not in self.checkgraph[src]) # no duplicateedges
            self.checkgraph[src].add(dst)
        smatfile.close()

        # load with random access, in-memory streaming, and disk-streaming
        self.bvg = bvg.BVGraph(self.graphname,1)
        self.bvgstream = bvg.BVGraph(self.graphname,0)
        self.bvgdisk = bvg.BVGraph(self.graphname,-1)

    def check_graph_simple(self,graph):
        self.assertEqual(graph.nverts, self.check_nnodes)
        self.assertEqual(graph.nedges, self.check_nedges)
    
    def test_simple(self):
        self.check_graph_simple(self.bvg)
        self.check_graph_simple(self.bvgstream)
        self.check_graph_simple(self.bvgdisk)

    def check_adjacency(self,graph):
        for src,neighs in graph.adjacency_iter():
            gneigh = set([dst for dst,_ in neighs.items()])
            self.assertSetEqual(self.checkgraph[src],gneigh)

    def test_adjacency(self):
        #self.check_adjacency(self.bvg)
        self.check_adjacency(self.bvgstream)
        #self.check_adjacency(self.bvgdisk)

    def check_edges(self, graph):
        for src, dst in graph.edges():
            self.assertIn(dst, self.checkgraph[src])

    def test_edges(self):
        self.check_edges(self.bvgstream)

    def check_edges_degree(self, graph):
        for src, dst, degree in graph.edges_and_degrees:
            self.assertIn(dst, self.checkgraph[src])
            self.assertEqual(len(checkgraph[src], degree))
    
    def test_edges_degree(self):
        self.check_edges_degree(self.bvgstream)

    def check_node_iterator(self, graph):
        for n in graph.nodes():
            self.assertIn(n, self.checkgraph)

    def test_node_iterator(self):
        self.check_node_iterator(self.bvg)
        self.check_node_iterator(self.bvgstream)
        self.check_node_iterator(self.bvgdisk)

    def check_iteration(self, graph):
        for v in graph:
            src = v.curr
            degree = v.degree
            self.assertEqual(degree, len(self.checkgraph[src]))
            gneigh = set([dst for dst in v])
            self.assertSetEqual(self.checkgraph[src], gneigh)

    def test_iteration(self):
        self.check_iteration(self.bvgstream)

    def check_random_iterator(self, graph, num):
        random.seed()
        sampled_nodes = random.sample(xrange(0, graph.nverts-1), num)
        for src, successors in graph.random_iterator(sampled_nodes):
            gneigh = set([dst for dst in successors])
            self.assertSetEqual(self.checkgraph[src], gneigh)
    
    def test_random_iterator(self):
        self.check_random_iterator(self.bvg, 100)

    def check_vertex_iterator(self, graph):
        for v in graph.vertices():
            src = v.curr
            degree = v.degree
            self.assertEqual(degree, len(self.checkgraph[src]))
            gneigh = set([dst for dst in v])
            self.assertSetEqual(self.checkgraph[src], gneigh)

    def test_vertex_iterator(self):
        self.check_vertex_iterator(self.bvgstream)

    def check_operator(self, graph):
        ingraph = 100
        notingraph = 100000000
        if ingraph in self.checkgraph:
            self.assertTrue(ingraph in graph)
        if notingraph not in self.checkgraph:
            self.assertTrue(notingraph not in graph)

    def test_operator(self):
        self.check_operator(self.bvg)
        self.check_operator(self.bvgstream)
        self.check_operator(self.bvgdisk)
    
    def check_length(self, graph):
        self.assertEqual(len(graph), len(self.checkgraph))
    
    def test_length(self):
        self.check_length(self.bvg)
        self.check_length(self.bvgstream)
        self.check_length(self.bvgdisk)

    def runTest(self):
        self.test_simple()
        self.test_adjacency()
        self.test_edges()
        self.test_iteration()
        self.test_random_iterator()
        self.test_operator()
        self.test_length()
        self.test_vertex_iterator()
        self.test_edges_degree
        self.test_node_iterator

## new an object without loading graph
#G = bvg.Graph()

## new an object with graph loaded
#data = sys.argv[1]
#G1 = bvg.BVGraph(data, 0)  # load with offset step = 0 (iteration)
#G2 = bvg.BVGraph(data, 1)  # load with offset step = 1 (random access)

#print G1.nverts
#testG = BVGraphTest(data)
#testG.setUp()
#testG.check_graph_simple(G1)

suite = unittest.TestLoader().loadTestsFromTestCase(BVGraphTest)
unittest.TextTestRunner(verbosity=2).run(suite)
