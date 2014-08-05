#!/usr/bin/python
from bvg import BVGraph
import sys
import random
import time

def iteration_test(g):
    # set nonzero iterator
    rval = g.nonzero_iterator()

    if rval == 1:
        print 'Construct nonzero iterator failed. Stop.'
        return 1
    
    g.iterator_next()

    while g.iterator_valid() == 1:
        a = g.iterator_outedges()
        d = len(a)
        curr = g.get_current_node()
        print 'node ' + str(curr) + ' has degree ' + str(d)
        g.iterator_next()

    g.iterator_free()

def random_access_test(g, test_num):
    # set random seed
    random.seed()

    # set random access iterator
    g.random_access_iterator()

    # count edge for evaluating performance
    edge_count = 0
    n = g.nverts
    m = g.nedges
    print n
    print m

    # sample a random test nodes
    sampled_nodes = random.sample(range(0, n-1), test_num)

    # set timer
    start = time.clock()

    # test test_num times
    for i in range(0, test_num):
        #node = random.randint(0, n)
        node = sampled_nodes[i]
        #node = i
        #print node
        #d = g.random_outdegree(node)
        links = g.random_successors(node)
        #print d

        edge_count += len(links)

        #for i in range(0, d):
            #print links[i]

    # count edges per second
    total_time = (time.clock() - start)
    eps = edge_count / total_time
    print 'Time: ' + str(total_time)

    # free the random iterator
    g.random_free()
    
    # return edges per second
    return eps

# load graph into BVGraph
filename = sys.argv[1]
g = BVGraph(filename)

# test sequential access
#iteration_test(g)

# test random access
# set test number
num = int(sys.argv[2])
eps = random_access_test(g, num)
print 'Edges per second: ' + str(eps)
