# file: libbvg.pyx

cimport clibbvg

class Vertex:
    """
    Base class for vertex.

    Each vertex object contains the information of current
    node, degree of the node, and the successors of the node.

    """
    def __init__(self, node, degree, successors):
        """Initialize the vertex object with node number, degree, 
        and successors.

        Parameters:
            node: node number
            degree: degree of this node
            successors: a list contains all the out-links of this node

        """

        self.curr = node
        self.degree = degree
        self.successors = successors

    def __iter__(self):
        """Iterate over the succssors of this vertex.
        Use the expression 'for n in V'.
        
        """
        for succ in self.successors:
            yield succ

    def items(self):
        """Iterate over the successors and attribute of this vertex.
        The attribute here is 1 for unweighted graph.
        Use the expression 'for succ, attr in V.items()'

        """
        for succ in self.successors:
            yield succ, 1

cdef class BVGraph:
    """Base class for Boldi-Vigna compressed graph.

    Parameters:
        filename: the filename of bvgraph without the extension.
            (Example: '../../data/harvard500')
        offset_step: load the graph with different ways.
            1 if load the offset into memory (default, for random access)
            0 if do not load the offset into memory (for streaming)
            -1 if load in disk

    Example:
        >>> G = bvg.BVGraph('../../data/harvard500')    # load the offset into memory
        >>> G = bvg.BVGraph('../../data/harvard500', 0) # load for streaming
        >>> G = bvg.BVGraph('../../data/harvard500', -1)    # load in disk

    """
    cdef clibbvg.bvgraph *g
    cdef clibbvg.bvgraph_iterator it
    cdef clibbvg.bvgraph_random_iterator ri
    cdef unsigned int d
    cdef int* links
    cdef char* filename
    
    ### constructor
    def __cinit__(self, filename, offset_step=None):
        """Initialize a graph with filename and offset steps.

        Parameters:
            filename: filename of the bvgraph data
            offset_step: load the graph with different ways.
                1 if load the offset into memory (default, for random access)
                0 if do not load the offset into memory (for streaming)
                -1 if load in disk

        """
        self.g = clibbvg.bvgraph_new()
        if self.g is NULL:
            raise MemoryError()

        if not isinstance(filename, str):
            raise NameError('Filename is not a string.')

        if isinstance(offset_step, int):
            rval = self.cload(filename, offset_step)
            self.filename = filename
            if rval == 1:
                raise IOError()
        else:
            rval = self.cload(filename, 1)
            self.filename = filename
            if rval == 1:
                raise IOError()
    
    ### destructor
    def __dealloc__(self):
        """Deallocate memory of BVGraph object.

        """
        if self.g is not NULL:
            clibbvg.bvgraph_free(self.g)

    ### for nodes and edges
    property nverts:
        """ Number of nodes in this graph.

        Example:
            >>> print G.nverts  # print number of nodes in this graph
        """
        def __get__(self):
            return self.g.n

    property nedges:
        """ Number of edges in this graph.

        Example:
            >>> print G.nedges  # print number of edges in this graph

        """
        def __get__(self):
            return self.g.m
    
    def __getitem__(self, n):
        """Return a dict of neighbors of node n.  Use the expression 'G[n]'.
        
        Parameter:
            n: given node number

        Example:
            >>> adj = G[1]  #return the adjacency dictionary for nodes connected to 1.

        """
        adj_dic = {}
        for node in self.successors(n):
            adj_dic[node] = {}
        return adj_dic

    def is_directed(self):
        """Return True if graph is directed, False otherwise."""
        return False

    def successors(self, x):
        """Random access to certain node x.

        Parameter:
            x: given node number

        Return a successor list of x.

        Example:
            >>> a = G.successors(x)
            >>> for v in a:
            ...     # v is a successor list of x

        """

        cdef clibbvg.bvgraph_random_iterator rit
        cdef int* successors
        cdef unsigned int degree
        clibbvg.bvgraph_random_access_iterator(self.g, &rit)
        clibbvg.bvgraph_random_successors(&rit, x, &successors, &degree)
        
        a = []
        for i in range(0, degree):
            a.append(successors[i])
        return a

    def out_degree(self, x):
        """Return the out degree of node x

        Parameter:
            x: given node number

        Example:
            >>> d = G.out_degree(x) # degree of x
        
        """
        cdef clibbvg.bvgraph_random_iterator rit
        cdef unsigned int degree
        clibbvg.bvgraph_random_access_iterator(self.g, &rit)
        if clibbvg.bvgraph_random_outdegree(&rit, x, &degree) == 0:
            return degree
        else:
            return -1

    ### iteration
    def __iter__(self):
        """Iterate over the nodes.  Use the expression 'for n in G'

        Example:
            >>> G = bvg.BVGraph('../../data/harvard500', 0)
            >>> for n in G:
            ...     # n is a Vertex object of current node of iteration

        """
        # declare and initialize a new iterator
        cdef clibbvg.bvgraph_iterator nit
        clibbvg.bvgraph_nonzero_iterator(self.g, &nit)
        
        cdef int* links
        cdef unsigned d

        while clibbvg.bvgraph_iterator_valid(&nit):
            # get successors and degree
            clibbvg.bvgraph_iterator_outedges(&nit, &links, &d)

            # copy links into list
            a = []
            for i in range(0, d):
                a.append(links[i])

            # new a vertex object and yield
            v = Vertex(nit.curr, d, a)
            yield v

            # iterate to next vertex
            clibbvg.bvgraph_iterator_next(&nit)

        # free iterator object
        clibbvg.bvgraph_iterator_free(&nit)

    def vertex(self, node):
        """Random access for vertex

        Parameter:
            node: given node number

        Example:
            >>> v = G.vertex(x)
            ...     # v.curr is x
            ...     # v.degree is number of successors in x
            >>> for neigh in v:
            ...     # (v.curr, neigh) is an edge
        
        """
 
        cdef clibbvg.bvgraph_random_iterator ri
        clibbvg.bvgraph_random_access_iterator(self.g, &ri)
        
        cdef int* links
        cdef unsigned d

        # get degree and successros
        clibbvg.bvgraph_random_successors(&ri, node, &links, &d)

        # copy links into list
        a = []
        for i in range(0, d):
            a.append(links[i])
        
        # free random iterator object
        clibbvg.bvgraph_random_free(&ri)

        # new an object and return
        v = Vertex(node, d, a)
        return v

    ## node id iterator
    def nodes(self):
        """A sequential iterator over all the nodes

        Example:
            >>> for v in G.nodes():
            ...     # v is a node

        """
 
        for i in xrange(0, self.nverts):
            yield i
   
    ## edge iterator
    def edges(self):
        """A sequential iterator over all the edges

        Example:
            >>> for e in G.edges():
            ...     # (e[0], e[1]) is an edge
            >>> for (src, dst) in self.edges():
            ...     # (src, dst) is an edge

        """
 
        # declare and initialize a new iterator
        cdef clibbvg.bvgraph_iterator it
        clibbvg.bvgraph_nonzero_iterator(self.g, &it)
        
        cdef int* links
        cdef unsigned d

        while clibbvg.bvgraph_iterator_valid(&it):
            # get successors and degree
            clibbvg.bvgraph_iterator_outedges(&it, &links, &d)
            
            src = it.curr
            for i in xrange(0, d):
                dst = links[i]
                e = src, dst
                yield e

            # iterate to next vertex
            clibbvg.bvgraph_iterator_next(&it)

        # free iterator object
        clibbvg.bvgraph_iterator_free(&it)

    ## edge-degree iterator
    def edges_and_degrees(self):
        """A sequential iterator over all the edges and degrees

        Example:
            >>> for e in self.edges_and_degrees():
            ...     # (e[0], e[1]) is an edge
            ...     # e[2] is the degree of e[0]
            >>> for (src, dst, degree) in self.edges_and_degrees():
            ...     # (src, dst) is an edge
            ...     # degree is number of links from src

        """
 
        # declare and initialize a new iterator
        cdef clibbvg.bvgraph_iterator it
        clibbvg.bvgraph_nonzero_iterator(self.g, &it)
        
        cdef int* links
        cdef unsigned d

        while clibbvg.bvgraph_iterator_valid(&it):
            # get successors and degree
            clibbvg.bvgraph_iterator_outedges(&it, &links, &d)
            
            src = it.curr
            for i in xrange(0, d):
                dst = links[i]
                e = src, dst, d
                yield e

            # iterate to next vertex
            clibbvg.bvgraph_iterator_next(&it)

        # free iterator object
        clibbvg.bvgraph_iterator_free(&it)

    ## vertex iterator
    def vertices(self):
        """A sequential iterator over all the vertices with information

        Example:
            >>> for v in self.vertices():
            ...     src = v.curr    # src is the node of current iteration
            ...     deg = v.degree  # deg is number of links of src
            ...     for neigh in v:
            ...         # (src, neigh) is an edge

        """
 
        # declare and initialize a new iterator
        cdef clibbvg.bvgraph_iterator nit
        clibbvg.bvgraph_nonzero_iterator(self.g, &nit)
        
        cdef int* links
        cdef unsigned d

        while clibbvg.bvgraph_iterator_valid(&nit):
            # get successors and degree
            clibbvg.bvgraph_iterator_outedges(&nit, &links, &d)
            
            # copy links into list
            a = []
            for i in range(0, d):
                a.append(links[i])
            
            # new a vertex object and yield
            v = Vertex(nit.curr, d, a)
            yield v

            # iterate to next vertex
            clibbvg.bvgraph_iterator_next(&nit)

        # free iterator object
        clibbvg.bvgraph_iterator_free(&nit)

    ## length function
    def __len__(self):
        """Return the number of nodes.  Use the expression 'len(G)'
        """
        return self.nverts

    ## in opertaor
    def __contains__(self, x):
        """Return True if x is a node, False otherwise.
        Use the expression 'n in G'

        Example:
            >>> G = bvg.BVGraph('../../data/harvard500')
            >>> 1 in G
            True

        """
        return isinstance(x, int) and x >= 0 and x < self.nverts

    ## load
    def load(self, filename, offset_step=1):
        """Loading with filename and offset steps

        Example:
            >>> G.load(filename, -1)    # loading in the disk rather than in memory
            >>> G.load(filename, 0)     # loading without offsets
            >>> G.load(filename, 1)     # loading with offsets

        """
        return self.cload(filename, offset_step)

    ## random iterator
    def random_iterator(self, node_list):
        """A random iterator iterates over given nodes.

        Should be used when BVGraph is loading with offset_steps=1

        Example:
            >>> nodes = [1, 2, 10, 20, 100, 200]
            >>> for n, successors in self.random_iterator(nodes):
            ...     for succ in xrange(0, len(successors)):
            ...         # succ is a successor from n
            ...         # (node, succ) is an edge
        """

        ## iterate over node_list
        cdef clibbvg.bvgraph_random_iterator ri
        cdef int* links
        cdef unsigned d
        clibbvg.bvgraph_random_access_iterator(self.g, &ri)

        for node in node_list:
            clibbvg.bvgraph_random_successors(&ri, node, &links, &d)
            
            # copy links into list
            a = []
            for i in range(0, d):
                a.append(links[i])
 
            yield node, a

        clibbvg.bvgraph_random_free(&ri)
    
    ## adjacency iter function
    def adjacency_iter(self):
        """A sequential iterator over nodes and all adjacency information

        The attribute returned for a node is always 1.

        Example:
            >>> for src,adj in self.adjacency_iter():
            ...     deg = len(adj)
            ...     for dst,attr in adj.items():
            ...         # (src,dst) is an edge.  
            ...         # attr is always 1
        """

        # declare and initialize a new iterator
        cdef clibbvg.bvgraph_iterator it
        clibbvg.bvgraph_nonzero_iterator(self.g, &it)
        
        cdef int* links
        cdef unsigned d

        while clibbvg.bvgraph_iterator_valid(&it):
            # get successors and degree
            clibbvg.bvgraph_iterator_outedges(&it, &links, &d)
            
            # copy links into list
            a = []
            for i in range(0, d):
                a.append(links[i])
            
            # new a vertex object
            v = Vertex(it.curr, d, a)
            yield it.curr, v

            # iterate to next vertex
            clibbvg.bvgraph_iterator_next(&it)

        # free iterator object
        clibbvg.bvgraph_iterator_free(&it)

    ### load function
    cdef int cload(self, char* filename, int offset_step):
        cdef int length = len(filename)
        return clibbvg.bvgraph_load(self.g, filename, length, offset_step)

    ### iterators
    cdef int cnonzero_iterator(self):
        return clibbvg.bvgraph_nonzero_iterator(self.g, &self.it)
    
    cdef int crandom_access_iterator(self):
        return clibbvg.bvgraph_random_access_iterator(self.g, &self.ri)

    ### for sequential access
    cdef int citerator_valid(self):
        return clibbvg.bvgraph_iterator_valid(&self.it)

    cdef int citerator_next(self):
        return clibbvg.bvgraph_iterator_next(&self.it)

    cdef int citerator_outedges(self):
        return clibbvg.bvgraph_iterator_outedges(&self.it, &self.links, &self.d)
    
    cdef int citerator_free(self):
        return clibbvg.bvgraph_iterator_free(&self.it)
    ######

    ### for random access
    cdef int crandom_outdegree(self, int x):
        cdef unsigned int d
        if clibbvg.bvgraph_random_outdegree(&self.ri, x, &d) == 0:
            return d
        else:
            return -1

    cdef int crandom_successors(self, int x):
        return clibbvg.bvgraph_random_successors(&self.ri, x, &self.links, &self.d)

    cdef int crandom_free(self):
        return clibbvg.bvgraph_random_free(&self.ri)
    ######

    ### interface for python
    def load(self, filename, offset_step):
        return self.cload(filename, offset_step)

    ### sequential access
    def nonzero_iterator(self):
        return self.cnonzero_iterator()

    def iterator_valid(self):
        return self.citerator_valid()

    def iterator_next(self):
        return self.citerator_next()

    def iterator_outedges(self):
        self.citerator_outedges()
        a = []
        for i in range(0, self.d):
            a.append(self.links[i])
        return a

    def iterator_free(self):
        return self.citerator_free()

    ### random access
    def random_access_iterator(self):
        return self.crandom_access_iterator()

    def random_outdegree(self, x):
        self.crandom_outdegree(x)
        return self.d

    def random_successors(self, x):
        self.crandom_successors(x)
       
        # use list to store
        a = []
        for i in range(0, self.d):
            a.append(self.links[i])
        return a

    def random_free(self):
        return self.crandom_free()
    
    ### some other functions
    def node_size(self):
        return self.g.n

    def edge_size(self):
        return self.g.m

    def get_degree(self):
        return self.d
    
    def get_current_node(self):
        return self.it.curr

## load graph
def load_graph(filename, offline=None, offsets=None):
    """Load the graph with filename and offset setting
    """
    G = BVGraph()
    if offline is True:
        G.load(filename, -1)
    elif offsets is True:
        G.load(filename, 1)
    else:
        G.load(filename, 0)
    return G

## load graph offline
def OfflineGraph(self, filename):
    """Loading filename in an offline mode

    """
    return BVGraph(filename, -1)


