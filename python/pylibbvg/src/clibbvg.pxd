# file: clibbvg.pxd
from libc.stdint cimport int64_t, uint64_t
cdef extern from "../include/bvgraph.h":
    ctypedef struct bvgraph:
        int64_t n
        int64_t m
        pass
    ctypedef struct bvgraph_iterator:
        int64_t curr
        pass
    ctypedef struct bvgraph_random_iterator:
        pass

    # initiailize functions
    bvgraph* bvgraph_new()
    void bvgraph_free(bvgraph* g)

    # load function
    int bvgraph_load(bvgraph* g, char *filename, unsigned int filenamelen, int offset_step)
    
    # iterator construct function
    int bvgraph_nonzero_iterator(bvgraph* g, bvgraph_iterator *i)
    int bvgraph_random_access_iterator(bvgraph* g, bvgraph_random_iterator *ri)
    
    # for sequential access
    int bvgraph_iterator_valid(bvgraph_iterator* i)
    int bvgraph_iterator_next(bvgraph_iterator* i)
    int bvgraph_iterator_outedges(bvgraph_iterator* i, int64_t** start, uint64_t* length)
    int bvgraph_iterator_free(bvgraph_iterator *i)

    int bvgraph_outdegree(bvgraph *g, int64_t x, uint64_t *d)
    int bvgraph_successors(bvgraph *g, int64_t x, int64_t** start, uint64_t *length)

    # for random access
    int bvgraph_random_outdegree(bvgraph_random_iterator *ri, int64_t x, uint64_t *d)
    int bvgraph_random_successors(bvgraph_random_iterator *ri, int64_t x, int64_t** start, uint64_t *length)
    int bvgraph_random_free(bvgraph_random_iterator *ri)

