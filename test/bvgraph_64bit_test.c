/**
 * @file bvgraph_64bit_test.c
 * Sequetially read a bvgraph, which contains 10000000000 nodes and 1000 edges. The edges are (1, 10000000000),
 * (2, 9999999999), ..., (1000, 9999999001).
 */

#include "bvgraph.h"
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

// disable all of the unsafe operation warnings
#ifdef _MSC_VER
#define inline __inline
#if _MSC_VER >= 1400
#pragma warning ( push )
#pragma warning ( disable: 4996 )
#endif /* _MSC_VER >= 1400 */
#endif /* _MSC_VER */

int main(int argc, char **argv)
{
    bvgraph graph = {0};
    bvgraph *g = &graph;
    
    char *filename;
    int filenamelen;
    int rval;
    int fastflag = 0;
    
    if (argc < 2) { fprintf(stderr, "Usage: bvgraph_64bit_test bvgraph_basename [fast]\n"); return (-1); }
    
    filename = argv[1];
    filenamelen = (int)strlen(filename);
    
    if (argc == 3) {
        fastflag = atoi(argv[2]);
    }
    
    rval = bvgraph_load(g, filename, filenamelen, -1);
    printf("#node = %" PRId64 ", #edges = %" PRId64 "\n", g->n, g->m);
    if (rval) { perror("error with partial load!"); }
    {
        bvgraph_iterator iter;
        int64_t *links = NULL;
        uint64_t d;
        for (bvgraph_nonzero_iterator(g, &iter);
           bvgraph_iterator_valid(&iter);
           bvgraph_iterator_next(&iter))
       {
            bvgraph_iterator_outedges(&iter, &links, &d);
            if (d > 0) {
                int64_t sum = 10000000001;
                if (sum - iter.curr != links[0] || d != 1)  {
                    printf("error when reading node = %" PRId64 "\n", iter.curr);
                    return (-1);
                }
            }
            if (fastflag && iter.curr > 1000) {
                break;
            }
       }
    }
    bvgraph_close(g);
    if (fastflag) {
        printf("Fast test of 64-bit bvgraph sequential read ... passed!\n");
    } else {
        printf("Testing 64-bit bvgraph sequential read ... passed!\n");
    }
    return 0;
}

