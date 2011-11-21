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

cdef class VertexIterator:
    cdef clibbvg.bvgraph *g

    # should throw g instead of loading graph again, but 
    # how to throw c variable as python object?
    def __init__(self, filename):
        self.g = clibbvg.bvgraph_new()
        cdef int length = len(filename)
        clibbvg.bvgraph_load(self.g, filename, length, 0)
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

cdef class EdgeIterator:
    cdef clibbvg.bvgraph *g
    def __init__(self, filename):
        self.g = clibbvg.bvgraph_new()
        cdef int length = len(filename)
        clibbvg.bvgraph_load(self.g, filename, length, 0)
    def __iter__(self):
        # declare and initialize a new iterator
        cdef clibbvg.bvgraph_iterator it
        clibbvg.bvgraph_nonzero_iterator(self.g, &it)
        
        cdef int* links
        cdef unsigned d

        while clibbvg.bvgraph_iterator_valid(&it):
            # get successors and degree
            clibbvg.bvgraph_iterator_outedges(&it, &links, &d)
            
            src = it.curr
            for i in range(0, d):
                dst = links[i]
                e = src, dst
                yield e

            # iterate to next vertex
            clibbvg.bvgraph_iterator_next(&it)

        # free iterator object
        clibbvg.bvgraph_iterator_free(&it)

cdef class EdgeDegreeIterator:
    cdef clibbvg.bvgraph *g
    def __init__(self, filename):
        self.g = clibbvg.bvgraph_new()
        cdef int length = len(filename)
        clibbvg.bvgraph_load(self.g, filename, length, 0)
    def __iter__(self):
        # declare and initialize a new iterator
        cdef clibbvg.bvgraph_iterator it
        clibbvg.bvgraph_nonzero_iterator(self.g, &it)
        
        cdef int* links
        cdef unsigned d

        while clibbvg.bvgraph_iterator_valid(&it):
            # get successors and degree
            clibbvg.bvgraph_iterator_outedges(&it, &links, &d)
            
            src = it.curr
            for i in range(0, d):
                dst = links[i]
                e = src, dst, d
                yield e

            # iterate to next vertex
            clibbvg.bvgraph_iterator_next(&it)

        # free iterator object
        clibbvg.bvgraph_iterator_free(&it)


class NodeIterator:
    def __init__(self, nverts):
        self.nverts = nverts
    def __iter__(self):
        for i in xrange(self.nverts):
            yield i

cdef class RandomIterator:
    cdef clibbvg.bvgraph *g
    cdef clibbvg.bvgraph_random_iterator ri

    def __init__(self, filename):
        self.g = clibbvg.bvgraph_new()
        cdef int length = len(filename)
        clibbvg.bvgraph_load(self.g, filename, length, 1)
        clibbvg.bvgraph_random_access_iterator(self.g, &self.ri)

    #def __iter__(self):
    def iterate(self, node_list):
        ## iterate over node_list
        cdef int* links
        cdef unsigned d
        cdef np.npy_intp shape[1]

        for node in node_list:
            clibbvg.bvgraph_random_successors(&self.ri, node, &links, &d)
            shape[0] = <np.npy_intp> d
            ndarray = np.PyArray_SimpleNewFromData(1, shape, np.NPY_INT, <void *> links)
            np.PyArray_UpdateFlags(ndarray, ndarray.flags.num | np.NPY_OWNDATA)

            yield node, ndarray

cdef class Graph:
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
        clibbvg.bvgraph_random_free(&self.ri)

        # new an object and return
        v = Vertex(node, d, ndarray)
        return v

    ## node id iterator
    def nodes(self):
        n = NodeIterator(self.nverts)
        return n
    
    ## edge iterator
    def edges(self):
        ei = EdgeIterator(self.filename)
        return ei

    ## edge-degree iterator
    def edges_and_degrees(self):
        edi = EdgeDegreeIterator(self.filename)
        return edi

    ## vertex iterator
    def vertices(self):
        vi = VertexIterator(self.filename)
        return vi

    ## length function
    def __len__(self):
        return self.nverts

    ## in opertaor
    def __contains__(self, x):
        return isinstance(x, int) and x in range(0, self.nverts)

    ## load graph offline (not supported currently)
    def OfflineGraph(self, filename):
        self.cload(filename, -1)

    ## load
    def load(self, filename, offset_step):
        return self.cload(filename, offset_step)

    ## random iterator
    def random_iterator(self):
        ri = RandomIterator(self.filename)
        return ri
    
    ## adjacency iter function
    def adjacency_iter(self):
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
    G = Graph()
    if offline is True:
        G.load(filename, -1)
    elif offsets is True:
        G.load(filename, 1)
    else:
        G.load(filename, 0)
    return G


