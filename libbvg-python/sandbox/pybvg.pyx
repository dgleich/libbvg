#from '../libbvg/include/bvgraph.h' cimport *
#from ../libbvg/include/bitfile.h cimport *

#cdef bvgraph

cdef extern from "../libbvg/include/bvgraph.h":
    struct bvgraph "bvg"
    struct bvgraph_random_iterator
    int bvgraph_load(bvgraph* g, char *filename, unsigned int filenamelen, int offset_step)

cdef class bvg:
    cdef bvgraph *g
    
    cdef cload(self, char* filename):
        length = len(filename)
        offset_step = 1
        bvgraph_load(self.g, filename, length, offset_step)

    cdef printn(self):
        num = self.g.n
        print num

#    def load(self, filename):
#        cload(self.g, filename)

#    def __init__(self):
#        a = 1
#g = []
#        initialize(self.g)


#    int load(bvgraph g, char *filename, unsigned int filenamelen, int offset_step):
#        return bvgraph_load(&g, filename, filenamelen, offset_step)

#cdef int load(char* filename):
#    bvgraph g

#def bvgraph_load(filename):
#    cdef struct bvgraph a
#    return load(&a, filename, len(filename), 1)
