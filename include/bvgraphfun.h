#ifndef LIBBVG_BVGRAPHFUN_H
#define LIBBVG_BVGRAPHFUN_H

/*
 * David Gleich
 * Copyright, Stanford University, 2007
 * 21 May 2007
 */

/**
 * @file bvgraphfun.h
 * Include a few functions with a bvgraph to make linear algebra easier
 */

#include "bvgraph.h"

int bvgraph_mult(bvgraph *g, double *x, double *y);
int bvgraph_transmult(bvgraph *g, double *x, double *y);
int bvgraph_diag(bvgraph *g, double *x);
int bvgraph_relax_sor(bvgraph *g, double *x, double w);
int bvgraph_sum_row(bvgraph *g, double *x);
int bvgraph_sum_col(bvgraph *g, double *x);
int bvgraph_csr(bvgraph *g, int* ai, int* aj);
int bvgraph_csr_large(bvgraph *g, size_t* ai, size_t* aj);

int bvgraph_substochastic_mult(bvgraph* g, double* x, double *y);
int bvgraph_substochastic_transmult(bvgraph* g, double* x, double *y);

#endif /* LIBBVG_BVGRAPHFUN_H */
