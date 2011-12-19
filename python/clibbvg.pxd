# file: clibbvg.pxd

cdef extern from "../include/bvgraph.h":
    ctypedef long long bvg_long_int
    ctypedef struct bvgraph:
        int n
        bvg_long_int m
        pass
    ctypedef struct bvgraph_iterator:
        int curr
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
    int bvgraph_iterator_outedges(bvgraph_iterator* i, int** start, unsigned int* length)
    int bvgraph_iterator_free(bvgraph_iterator *i)

    int bvgraph_outdegree(bvgraph *g, int x, unsigned int *d)
    int bvgraph_successors(bvgraph *g, int x, int** start, unsigned int *length)

    # for random access
    int bvgraph_random_outdegree(bvgraph_random_iterator *ri, int x, unsigned int *d)
    int bvgraph_random_successors(bvgraph_random_iterator *ri, int x, int** start, unsigned int *length)
    int bvgraph_random_free(bvgraph_random_iterator *ri)

