libbvg
======

This library implements a decoder for a Boldi-Vigna 
graph structure, a highly compressed means of storing
web-graphs.  The library is written in pure C.
It includes a Matlab wrapper, and a means to implement
multi-threaded graph algorithms.

Features
--------

* In-memory and on-disk graph storage
* Sequential iteration over graph edges
* Parallel iteration over graph edges

Coming soon: random access to graph edges.

Synopsis
--------

    bvgraph g = {0};
    bvgraph_load(&g, "wb-cs.stanford", 14, 0);
    printf("nodes = %i\n", g.n);
    printf("edges = %i\n", g.m);

    bvgraph_iterator git;
    bvgraph_nonzero_iterator(&g, &git);

    for (; bvgraph_iterator_valid(&git); bvgraph_iterator_next(&git)) {
        int *links; unsigned int d;
        bvgraph_iterator_outedges(&git, &links, &d);
        printf("node %i has degree %d\n", git.curr);
        for (int i; i<d; ++i) {
            printf("node %i links to node %i\n", links[i]);
        }
    }
    bvgraph_iterator_free(&git);
    
    bvgraph_close(&g);

License
-------

GPL version 2.

Copyright by David F. Gleich, 2007-2010
