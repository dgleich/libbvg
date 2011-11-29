# file: libbvg.pyx

cimport clibbvg
import numpy as np
cimport numpy as np

np.import_array()

class Vertex:
    def __init__(self, node, degree, successors):
        self.curr = node
        self.degree = degree
        self.successors = successors
    def __iter__(self):
        for succ in self.successors:
            yield succ
    def items(self):
        for succ in self.successors:
            yield succ, 1

cdef class BVGraph:
    cdef clibbvg.bvgraph *g
    cdef clibbvg.bvgraph_iterator it
    cdef clibbvg.bvgraph_random_iterator ri
    cdef unsigned int d
    cdef int* links
    cdef char* filename
    
    ### constructor
    def __cinit__(self, filename=None, offset_step=None):
        self.g = clibbvg.bvgraph_new()
        if self.g is NULL:
            raise MemoryError()

        if isinstance(filename, str) & isinstance(offset_step, int):
            rval = self.cload(filename, offset_step)
            self.filename = filename
            if rval == 1:
                raise IOError()
    
    ### destructor
    def __dealloc__(self):
        if self.g is not NULL:
            clibbvg.bvgraph_free(self.g)

    ### for nodes and edges
    property nverts:
        def __get__(self):
            return self.g.n

    property nedges:
        def __get__(self):
            return self.g.m

    ################## new interfaces
    def successors(self, x):
        """Random access to certain node x.

        Return a successor list of x.

        Example:
            a = self.successors(x)
            for v in a:
                # v is a successor of x

        """
 
        cdef clibbvg.bvgraph_random_iterator rit
        cdef int* successors
        cdef unsigned int degree
        clibbvg.bvgraph_random_access_iterator(self.g, &rit)
        clibbvg.bvgraph_random_successors(&rit, x, &successors, &degree)
        cdef np.npy_intp shape[1]
        shape[0] = <np.npy_intp> degree

        ndarray = np.PyArray_SimpleNewFromData(1, shape, np.NPY_INT, <void *> successors)
        np.PyArray_UpdateFlags(ndarray, ndarray.flags.num | np.NPY_OWNDATA)
        
        return ndarray

    def out_degree(self, x):
        cdef clibbvg.bvgraph_random_iterator rit
        cdef unsigned int degree
        clibbvg.bvgraph_random_access_iterator(self.g, &rit)
        if clibbvg.bvgraph_random_outdegree(&rit, x, &degree) == 0:
            return degree
        else:
            return -1

    ### iteration
    def __iter__(self):
        # declare and initialize a new iterator
        cdef clibbvg.bvgraph_iterator nit
        clibbvg.bvgraph_nonzero_iterator(self.g, &nit)
        
        cdef int* links
        cdef unsigned d
        cdef np.npy_intp shape[1]

        while clibbvg.bvgraph_iterator_valid(&nit):
            # get successors and degree
            clibbvg.bvgraph_iterator_outedges(&nit, &links, &d)
            
            # copy links into numpy array
            shape[0] = <np.npy_intp> d
            ndarray = np.PyArray_SimpleNewFromData(1, shape, np.NPY_INT, <void *> links)
            np.PyArray_UpdateFlags(ndarray, ndarray.flags.num | np.NPY_OWNDATA)

            # new a vertex object and yield
            v = Vertex(nit.curr, d, ndarray)
            yield v

            # iterate to next vertex
            clibbvg.bvgraph_iterator_next(&nit)

        # free iterator object
        clibbvg.bvgraph_iterator_free(&nit)

    def vertex(self, node):
        """Random access for vertex

        Example:
            v = self.vertex(x)
            # v.curr is x
            # v.degree is number of successors in x
            for neigh in v:
                # (v.curr, neigh) is an edge

        """
 
        cdef clibbvg.bvgraph_random_iterator ri
        clibbvg.bvgraph_random_access_iterator(self.g, &ri)
        
        cdef int* links
        cdef unsigned d

        # get degree and successros
        clibbvg.bvgraph_random_successors(&ri, node, &links, &d)

        # copy to numpy array
        cdef np.npy_intp shape[1]
        shape[0] = <np.npy_intp> d
        ndarray = np.PyArray_SimpleNewFromData(1, shape, np.NPY_INT, <void *> links)
        np.PyArray_UpdateFlags(ndarray, ndarray.flags.num | np.NPY_OWNDATA)

        # free random iterator object
        clibbvg.bvgraph_random_free(&ri)

        # new an object and return
        v = Vertex(node, d, ndarray)
        return v

    ## node id iterator
    def nodes(self):
        """A sequential iterator over all the nodes

        Example:
            for v in self.nodes():
                # v is a node

        """
 
        for i in xrange(0, self.nverts):
            yield i
   
    ## edge iterator
    def edges(self):
        """A sequential iterator over all the edges

        Example:
            for e in self.edges():
                # (e[0], e[1]) is an edge
            for (src, dst) in self.edge():
                # (src, dst) is an edge

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
            for e in self.edges_and_degrees():
                # (e[0], e[1]) is an edge
                # e[2] is the degree of e[0]
            for (src, dst, degree) in self.edges_and_degrees():
                # (src, dst) is an edge
                # degree is number of links from src

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
            for v in self.vertices():
                src = v.curr    # src is the node of current iteration
                deg = v.degree  # deg is number of links of src
                for neigh in v:
                    # (src, neigh) is an edge

        """
 
        # declare and initialize a new iterator
        cdef clibbvg.bvgraph_iterator nit
        clibbvg.bvgraph_nonzero_iterator(self.g, &nit)
        
        cdef int* links
        cdef unsigned d
        cdef np.npy_intp shape[1]

        while clibbvg.bvgraph_iterator_valid(&nit):
            # get successors and degree
            clibbvg.bvgraph_iterator_outedges(&nit, &links, &d)
            
            # copy links into numpy array
            shape[0] = <np.npy_intp> d
            ndarray = np.PyArray_SimpleNewFromData(1, shape, np.NPY_INT, <void *> links)
            np.PyArray_UpdateFlags(ndarray, ndarray.flags.num | np.NPY_OWNDATA)

            # new a vertex object and yield
            v = Vertex(nit.curr, d, ndarray)
            yield v

            # iterate to next vertex
            clibbvg.bvgraph_iterator_next(&nit)

        # free iterator object
        clibbvg.bvgraph_iterator_free(&nit)

    ## length function
    def __len__(self):
        return self.nverts

    ## in opertaor
    def __contains__(self, x):
        return isinstance(x, int) and x in xrange(0, self.nverts)

    ## load graph offline
    def OfflineGraph(self, filename):
        """Loading filename in an offline mode

        """
        self.cload(filename, -1)

    ## load
    def load(self, filename, offset_step):
        """Loading with filename and offset steps

        Example:
            G.load(filename, -1)    # loading in the disk rather than in memory
            G.load(filename, 0)     # loading without offsets
            G.load(filename, 1)     # loading with offsets

        """
        return self.cload(filename, offset_step)

    ## random iterator
    def random_iterator(self, node_list):
        """A random iterator iterates over given nodes

        Should be used when BVGraph is loading with offset_steps=1

        Example:
            nodes = [1, 2, 10, 20, 100, 200]
            for n, successors in self.random_iterator(nodes):
                for succ in xrange(0, len(successors)):
                    # succ is a successor from n
                    # (node, succ) is an edge
        """

        ## iterate over node_list
        cdef clibbvg.bvgraph_random_iterator ri
        cdef int* links
        cdef unsigned d
        cdef np.npy_intp shape[1]
        clibbvg.bvgraph_random_access_iterator(self.g, &ri)

        for node in node_list:
            clibbvg.bvgraph_random_successors(&ri, node, &links, &d)

            shape[0] = <np.npy_intp> d
            ndarray = np.PyArray_SimpleNewFromData(1, shape, np.NPY_INT, <void *> links)
            np.PyArray_UpdateFlags(ndarray, ndarray.flags.num | np.NPY_OWNDATA)

            yield node, ndarray

        clibbvg.bvgraph_random_free(&ri)
    
    ## adjacency iter function
    def adjacency_iter(self):
        """A sequential iterator over nodes and all adjacency information

        The attribute returned for a node is always 1.

        Example:
            for src,adj in self.adjacency_iter():
                deg = len(adj)
                for dst,attr in adj.items():
                    # (src,dst) is an edge.  
                    # attr is always 1
        """

        # declare and initialize a new iterator
        cdef clibbvg.bvgraph_iterator it
        clibbvg.bvgraph_nonzero_iterator(self.g, &it)
        
        cdef int* links
        cdef unsigned d
        cdef np.npy_intp shape[1]

        while clibbvg.bvgraph_iterator_valid(&it):
            # get successors and degree
            clibbvg.bvgraph_iterator_outedges(&it, &links, &d)
            
            # copy to numpy array
            shape[0] = <np.npy_intp> d
            ndarray = np.PyArray_SimpleNewFromData(1, shape, np.NPY_INT, <void *> links)
            np.PyArray_UpdateFlags(ndarray, ndarray.flags.num | np.NPY_OWNDATA)

            # new a vertex object
            v = Vertex(it.curr, d, ndarray)
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
        cdef np.npy_intp shape[1]
        shape[0] = <np.npy_intp> self.d

        ndarray = np.PyArray_SimpleNewFromData(1, shape, np.NPY_INT, <void *> self.links)
        np.PyArray_UpdateFlags(ndarray, ndarray.flags.num | np.NPY_OWNDATA)
        return ndarray
        
        # try numpy array, slow!
        #a = np.zeros((self.d, 1))
        #for i in range(0, self.d):
        #    a[i] = self.links[i]

        # use list to store
        #a = []
        #for i in range(0, self.d):
        #    a.append(self.links[i])
        #return a

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
    G = BVGraph()
    if offline is True:
        G.load(filename, -1)
    elif offsets is True:
        G.load(filename, 1)
    else:
        G.load(filename, 0)
    return G
