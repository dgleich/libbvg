/**
 * This test case is turned off by default since it requires a lot of memory for random access of the graph.
 */

#include "bvgraph.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
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

    int i, rval;

    if (argc < 2) { fprintf(stderr, "Usage: bvgraph_test bvgraph_basename\n"); return (-1); }
    
    filename = argv[1];
    filenamelen = (int)strlen(filename);
    rval = bvgraph_load(g, filename, filenamelen, 1);
    printf("#nodes = %"PRId64", #edges = %"PRId64"\n", g->n, g->m);
    if (rval) { perror("error with full load!"); }
    {
        int64_t node_num = 10000000000;
        int64_t curr, d;
        int64_t *suc = NULL;
        bvgraph_random_iterator riter;
        for (i = 0; i < 1000; i ++) {
            srand( time(NULL) );
            curr = rand() % node_num;
            bvgraph_random_access_iterator(g, &riter);
            bvgraph_random_successors(&riter, curr, &suc, &d);
            if (suc[0] != curr + 1) {
                printf("64 bit line graph test failed on node %"PRId64"\n", curr);
                return (-1);
            }
        }
        bvgraph_random_free(&riter);
    }
    bvgraph_close(g);
    printf("Testing 64-bit bvgraph random read ... passed!\n");
    return 0;
}