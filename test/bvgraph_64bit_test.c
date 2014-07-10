/**
 * @file bvgraph_64bit_test.c
 * Sequetially read a bvgraph, which contains 10000000000 nodes and 1000 edges. The edges are (1, 10000000000),
 * (2, 9999999999), ..., (1000, 9999999001).
 */

#include "bvgraph.h"
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>

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
    
    if (argc < 2) { fprintf(stderr, "Usage: bvgraph_64bit_test bvgraph_basename\n"); return (-1); }
    
    filename = argv[1];
    filenamelen = (int)strlen(filename);
    
    int rval;
    /*rval = bvgraph_load(g, filename, filenamelen, -1);
    if (rval) { perror("error with initial load!"); }
    {
        size_t memrequired, offset_buff;
        bvgraph_required_memory(g, 1, &memrequired, &offset_buff);
        printf("the graph %s requires %llu bytes to load into memory, offset_buff=%llu\n", filename, (long long)memrequired, (long long)offset_buff);
    }
    bvgraph_close(g);*/
    clock_t begin, end;
    double time_spent;
    begin = clock();
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
            if (d > 0 && iter.curr <= 1000) {
                int64_t sum = 10000000001;
                if (sum - iter.curr != links[0]) {
                    printf("error when reading node = %" PRId64 "\n", iter.curr);
                    return (-1);
                }
            }
            else if (iter.curr > 1000) {
                break;
            }       
       }
    }
    bvgraph_close(g);
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("64-bit sequential test took %f secs, %ju clock cycles\n", time_spent, (uintmax_t)(end - begin));
    printf("Testing 64-bit bvgraph sequential read ... passed!\n");
    return 0;
}

