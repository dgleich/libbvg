/* 
 * David Gleich
 * Copyright, Stanford University, 2007
 * 20 May 2007
 */

/**
 * @file bvgraphfun.c
 * Routines to manipulate a bvgraph like a matrix.
 */

#include "bvgraph.h"

#include <string.h>

int bvgraph_mult(bvgraph *g, double *x, double *y)
{
    bvgraph_iterator iter;
    int *links; unsigned int i, d;
    for (bvgraph_nonzero_iterator(g, &iter); 
         bvgraph_iterator_valid(&iter); 
         bvgraph_iterator_next(&iter))
    {
        double v = 0;
        bvgraph_iterator_outedges(&iter, &links, &d);
        for (i = 0; i < d; i++) {
            v += x[links[i]];
        }
        *(y++) = v;
    }
    bvgraph_iterator_free(&iter);
    return (0);
}

int bvgraph_transmult(bvgraph *g, double *x, double *y)
{
    bvgraph_iterator iter;
    int *links; unsigned int i, d;
    memset(y, 0, sizeof(double)*g->n);
    for (bvgraph_nonzero_iterator(g, &iter); 
         bvgraph_iterator_valid(&iter); 
         bvgraph_iterator_next(&iter))
    {
        bvgraph_iterator_outedges(&iter, &links, &d);
        for (i = 0; i < d; i++) {
            y[links[i]] += x[iter.curr];
        }
    }
    bvgraph_iterator_free(&iter);
    return (0);
}

int bvgraph_diag(bvgraph *g, double *x)
{
    bvgraph_iterator iter;
    int *links; unsigned int i, d;
    for (bvgraph_nonzero_iterator(g, &iter); 
         bvgraph_iterator_valid(&iter); 
         bvgraph_iterator_next(&iter))
    {
        double v = 0;
        bvgraph_iterator_outedges(&iter, &links, &d);
        for (i = 0; i < d; i++) {
            if (links[i] == iter.curr) {
                v = 1.0;
            }
        }
        *(x++) = v;
    }
    bvgraph_iterator_free(&iter);
    return (0);
}

int bvgraph_relax_sor(bvgraph *g, double *x, double w)
{
    return (0);
}

int bvgraph_sum_row(bvgraph *g, double *x)
{
    bvgraph_iterator iter;
    unsigned int d;
    for (bvgraph_nonzero_iterator(g, &iter); 
         bvgraph_iterator_valid(&iter); 
         bvgraph_iterator_next(&iter))
    {
        bvgraph_iterator_outedges(&iter, NULL, &d);
        *(x++) = (double)d;
    }
    bvgraph_iterator_free(&iter);
    return (0);
}

int bvgraph_sum_col(bvgraph *g, double *x)
{
    bvgraph_iterator iter;
    int *links; unsigned int i, d;
    for (bvgraph_nonzero_iterator(g, &iter); 
         bvgraph_iterator_valid(&iter); 
         bvgraph_iterator_next(&iter))
    {
        bvgraph_iterator_outedges(&iter, &links, &d);
        for (i = 0; i < d; i++) {
            x[links[i]]++;
        }
    }
    bvgraph_iterator_free(&iter);
    return (0);
}
