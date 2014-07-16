/* 
 * David Gleich
 * Copyright, Stanford University, 2007
 * 20 May 2007
 */

/**
 * @file bvgraphfun.c
 * Routines to manipulate a bvgraph like a matrix.
 */

/** History
 * 3 September 2007
 * Added bvgraph_csr and bvgraph_csr_large to convert to a CSR representation.
 * Added bvgraph_substochastic_mult and bvgraph_substochastic_transmult
 *    to compute the stochastic products.
 * 
 *  2008-01-24:Added error checking and more comments
 * 
 * 4 March 2008
 * Added compensated summation
 */

#include "bvgraph.h"

/**
 * Define macros for compensated summation
 * 
 * given y[2] (a vector of length 2 of summation values)
 *       x        (the new summand)
 *       t        (a temp variable)
 * CSUM(x,svals,t,z) "computes y+= x" with compensated summation
 */
// y[0] = sum; y[1] = e 
#define CSUM2(x,y1,y2,t,z) { t=y1; z=(x)+y2; y1=t+z; y2=(t-y1)+z; }
#define CSUM(x,y,t,z) { t=y[0]; z=(x)+y[1]; y[0]=t+z; y[1]=(t-y[0])+z; }
#define FCSUM2(y1,y2) (y1+y2)
#define FCSUM(y) (y[0]+y[1])

#include <string.h>

/**
 * Computes a matrix vector product y = A*x 
 *
 * @param g the bvgraph structure
 * @param x the vector x
 * @param y the output vector y
 * @return 0 if successful
 */
int bvgraph_mult(bvgraph *g, double *x, double *y)
{
    bvgraph_iterator iter;
    int64_t *links; uint64_t i, d;
    int rval = bvgraph_nonzero_iterator(g, &iter);
    if (rval != 0) { return rval; } 
    for (; bvgraph_iterator_valid(&iter); 
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

/**
 * Computes a matrix vector product y = A'*x 
 *
 * @param g the bvgraph structure
 * @param x the vector x
 * @param y the output vector y
 * @return 0 if successful
 */
int bvgraph_transmult(bvgraph *g, double *x, double *y)
{
    bvgraph_iterator iter;
    int64_t *links; uint64_t i, d;
    int rval = bvgraph_nonzero_iterator(g, &iter);
    if (rval != 0) { return rval; }
    memset(y, 0, sizeof(double)*g->n); 
    for (; bvgraph_iterator_valid(&iter); 
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

/**
 * Extract the entries along the diagonal of the matrix. 
 *
 * @param g the bvgraph structure
 * @param x the vector of diagonal elements (size g.n)
 * @return 0 if successful
 */
int bvgraph_diag(bvgraph *g, double *x)
{
    bvgraph_iterator iter;
    int64_t *links; uint64_t i, d;
    int rval = bvgraph_nonzero_iterator(g, &iter);
    if (rval != 0) { return rval; } 
    for (; bvgraph_iterator_valid(&iter); 
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

/**
 * Compute the sum along rows of the matrix, i.e. x = A*ones(g.n,1), 
 * but efficiently.
 *
 * @param g the bvgraph structure
 * @param x the vector of row sums
 * @return 0 if successful 
 */
int bvgraph_sum_row(bvgraph *g, double *x)
{
    bvgraph_iterator iter;
    uint64_t d;
    int rval = bvgraph_nonzero_iterator(g, &iter);
    if (rval != 0) { return rval; } 
    for (; bvgraph_iterator_valid(&iter); 
         bvgraph_iterator_next(&iter))
    {
        bvgraph_iterator_outedges(&iter, NULL, &d);
        *(x++) = (double)d;
    }
    bvgraph_iterator_free(&iter);
    return (0);
}

/**
 * Compute the sum along columns of the matrix, i.e. x = ones(g.n,1)'*A, 
 * but efficiently.
 *
 * @param g the bvgraph structure
 * @param x the vector of column sums
 * @return 0 if successful
 */
int bvgraph_sum_col(bvgraph *g, double *x)
{
    bvgraph_iterator iter;
    int64_t *links; uint64_t i, d;
    int rval = bvgraph_nonzero_iterator(g, &iter);
    if (rval != 0) { return rval; } 
    for (; bvgraph_iterator_valid(&iter); 
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

int bvgraph_csr(bvgraph *g, int* ai, int* aj)
{
    bvgraph_iterator iter;
    int64_t *links; uint64_t i, d;
    int rval = bvgraph_nonzero_iterator(g, &iter);
    int entry = 0;
    *ai++ = entry;
    
    if (rval != 0) { return rval; } 
    for (; bvgraph_iterator_valid(&iter); 
         bvgraph_iterator_next(&iter))
    {
        bvgraph_iterator_outedges(&iter, &links, &d);
        for (i = 0; i < d; i++) {
            *aj++ = links[i];
        }
        entry += d;
        *ai++ = entry;
    }
    bvgraph_iterator_free(&iter);
    return (0);
}


int bvgraph_csr_large(bvgraph *g, size_t* ai, size_t* aj)
{
    bvgraph_iterator iter;
    int64_t *links; uint64_t i, d;
    int rval = bvgraph_nonzero_iterator(g, &iter);
    size_t entry = 0;
    *ai++ = entry;
    
    if (rval != 0) { return rval; } 
    for (; bvgraph_iterator_valid(&iter); 
         bvgraph_iterator_next(&iter))
    {
        bvgraph_iterator_outedges(&iter, &links, &d);
        for (i = 0; i < d; i++) {
            *aj++ = (size_t)links[i];
        }
        entry += d;
        *ai++ = entry;
    }
    bvgraph_iterator_free(&iter);
    return (0);
}

/**
 * Computes a substochastic matrix vector product
 * y = (D^+ A) x 
 * efficiently without storing a vector of out-degrees.
 *
 * @param g the bvgraph structure
 * @param x the vector x
 * @param y the output vector y
 * @return 0 if successful
 */
int bvgraph_substochastic_mult(bvgraph *g, double *x, double *y)
{
    bvgraph_iterator iter;
    int64_t *links; uint64_t i, d;
    int rval = bvgraph_nonzero_iterator(g, &iter);
    if (rval != 0) { return rval; } 
    for (; bvgraph_iterator_valid(&iter); 
         bvgraph_iterator_next(&iter))
    {
        double v = 0;
        bvgraph_iterator_outedges(&iter, &links, &d);
        for (i = 0; i < d; i++) {
            v += x[links[i]]/(double)d;
        }
        *(y++) = v;
    }
    bvgraph_iterator_free(&iter);
    return (0);
}

/**
 * Computes a substochastic tranpose matrix vector product
 * y = (D^+ A)^T x 
 * efficiently without storing a vector of out-degrees.
 *
 * @param g the bvgraph structure
 * @param x the vector x
 * @param y the output vector y
 * @return 0 if successful
 */
int bvgraph_substochastic_transmult(bvgraph *g, double *x, double *y)
{
    bvgraph_iterator iter;
    int64_t *links; uint64_t i, d;
    int rval = bvgraph_nonzero_iterator(g, &iter);
    if (rval != 0) { return rval; } 
    memset(y, 0, sizeof(double)*g->n);
    for (; bvgraph_iterator_valid(&iter); 
         bvgraph_iterator_next(&iter))
    {
        bvgraph_iterator_outedges(&iter, &links, &d);
        for (i = 0; i < d; i++) {
            y[links[i]] += x[iter.curr]/((double)d);
        }
    }
    bvgraph_iterator_free(&iter);
    return (0);
}

/**
 * Compute the sum along rows of the matrix, i.e. x = P*ones(g.n,1), 
 * but efficiently
 *
 * @param g the bvgraph structure
 * @param x the vector of row sums
 * @return 0 if successful 
 */
int bvgraph_substochastic_sum_row(bvgraph *g, double *x)
{
    bvgraph_iterator iter; register double y1,y2,t,z,id;
    y1 = 0.0;
    y2 = 0.0;
    uint64_t d;
    int rval = bvgraph_nonzero_iterator(g, &iter);
    if (rval != 0) { return rval; } 
    for (; bvgraph_iterator_valid(&iter); 
         bvgraph_iterator_next(&iter))
    {
        bvgraph_iterator_outedges(&iter, NULL, &d);
        id=1.0/(double)d;
        while (d-->0) { CSUM2(id,y1,y2,t,z); } // implement compensated sum
        *(x++) = FCSUM2(y1,y2);
    }
    bvgraph_iterator_free(&iter);
    return (0);
}

/**
 * Compute the sum along columns of the matrix, i.e. x = ones(g.n,1)'*A, 
 * but efficiently.
 *
 * @param g the bvgraph structure
 * @param x the vector of column sums
 * @return 0 if successful
 */
int bvgraph_substochastic_sum_col(bvgraph *g, double *x)
{
    bvgraph_iterator iter;
    int64_t *links; uint64_t i, d;
    int rval = bvgraph_nonzero_iterator(g, &iter);
    if (rval != 0) { return rval; } 
    for (; bvgraph_iterator_valid(&iter); 
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
