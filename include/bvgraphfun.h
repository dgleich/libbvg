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

#endif /* LIBBVG_BVGRAPHFUN_H */
