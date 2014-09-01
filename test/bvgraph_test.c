/* 
 * David Gleich
 * Copyright, Stanford University, 2007
 * 17 May 2007
 */

/**
 * @file bvgraph_test.c
 * Read a bvgraph from a file and write its data to stdout
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
    int64_t i;

    if (argc < 2) { fprintf(stderr, "Usage: bvgraph_test bvgraph_basename\n"); return (-1); }

    filename = argv[1];
    filenamelen = (int)strlen(filename);

    rval = bvgraph_load(g, filename, filenamelen, -1);
    if (rval) { perror("error with initial load!"); }
    {
        size_t memrequired;
        bvgraph_required_memory(g, 0, &memrequired, NULL);
        printf("the graph %s requires %zu bytes to load into memory\n", filename, memrequired);
    }
    bvgraph_close(g);

    rval = bvgraph_load(g, filename, filenamelen, 0);
    if (rval) { perror("error with full load!"); }
    {
        bvgraph_iterator iter;
        int64_t *links = NULL;
        uint64_t d;
        // initialize a vector of column sums
        int64_t *colsum = malloc(sizeof(int64_t)*g->n);
        int64_t *colsum2 = malloc(sizeof(int64_t)*g->n);
        int rep;
        memset(colsum, 0, sizeof(int64_t)*g->n);
        for (bvgraph_nonzero_iterator(g, &iter); 
             bvgraph_iterator_valid(&iter); 
             bvgraph_iterator_next(&iter))
        {
            bvgraph_iterator_outedges(&iter, &links, &d);
            for (i = 0; i < d; i++) {
                colsum[links[i]]++;
            }
        }
        bvgraph_iterator_free(&iter);
        for (rep = 0; rep < 10000; rep++) {
            memset(colsum2, 0, sizeof(int64_t)*g->n);
            for (bvgraph_nonzero_iterator(g, &iter); 
                 bvgraph_iterator_valid(&iter); 
                 bvgraph_iterator_next(&iter))
            {
                bvgraph_iterator_outedges(&iter, &links, &d);
                for (i = 0; i < d; i++) {
                    colsum2[links[i]]++;
                }
            }
            bvgraph_iterator_free(&iter);
            for (i=0; i < g->n; i++) {
                if (colsum2[i] != colsum[i]) {
                    fprintf(stderr, "error, column sum of column %"PRId64 "is not correct (%"PRId64" =? %"PRId64")", 
                        i, colsum[i], colsum2[i]);
                    perror("colsum error!");
                }
            }
        }
        free(colsum);
        free(colsum2);
    }
    bvgraph_close(g);

    for (i = 0; i < 10000000; i++) {
        rval = bvgraph_load(g, filename, filenamelen, 0);
        bvgraph_close(g);
    }
}
