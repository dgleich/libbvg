import libbvg as bvg

# Setup basic metadata
G = bvg.Graph(filename)

# done
G.successors(i) # get the succesors for node i
G.out_degree(i) # get the out-degree of node i

# done
G.nverts
G.nedges

# done
G.vertex(j) # returns a structure for the vertex at j

# done
for v in G:
   # equivalent to v = G.vertex(j) for xrange(G.nverts)
   src = v.curr
   deg = v.degree
   for neigh in v:
       dst = neigh
       # now (src,dst) is an edge

# done
R1 = G.random_iterator()
R2 = G.random_iterator()

# all done
# same as network X
G.edges() # returns a thread-safe edge iterator
G.edges_and_degrees() # returns a thread-safe edge+degree iterator
G.vertices() # returns a thread-safe vertex iterator, same as __iter__()

G.nodes() # returns a node id iterator, very dull, just xrange(G.nverts)


# R1 and R2 can be used safely in separate threads
for (src,dst) in G.edges():
   # iterate only over the edges of the graph

for (src,dst,srcdeg) = G.edges_and_degrees():
   # iterate over edges and degrees

class MyTest:
   def __iter__(self):
       i = 0
       while i < 5:
           yield i
           i += 1



# TODO
G = bvg.OfflineGraph(filename)

# done
G = bvg.load_graph(filename,offline=True)
G = bvg.load_graph(filename,offsets=True)

#
G = bvg.Graph() # loads offsets and graph into memory if possible
G = bvg.OfflineGraph() # doesn't load anything into memory

also

#done
x = 0
if x in G:
   # return true if x is an int-type and 0 \le 0 \le G.nverts

# done
len(G) # return G.nverts

# done
for n, v in G.adjacency_iter():
   for nbr, attr = v.items()


# so the vertex class returned by G.vertex should have an .items() function
# and .iteritems() function
