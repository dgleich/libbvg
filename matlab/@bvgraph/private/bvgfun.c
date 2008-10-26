/* 
 * David Gleich
 * Copyright, Stanford University, 2007
 * 21 May 2007
 */

/**
 * @file bvgfun.c
 * Routines to manipulate a bvgraph like a matrix in matlab.
 */

/*
 * 1 September 2007
 * Added the diag_bvgraph function to get the diagonal of a matrix
 *
 * 3 September 2007
 * Added the sparse_bvgraph function to get the sparse matrix form.
 */

#include "bvgraph.h"
#include "bvgraphfun.h"

#include <string.h>

#include <mex.h>
#include <matrix.h>

#if MX_API_VER < 0x07030000
typedef int mwIndex;
typedef int mwSize;
#endif // MX_API_VER

int get_string_arg(const mxArray* arg, const char **str, mwSize *len)
{
    mwSize buflen;
    
    /* input must be a string */
    if ( mxIsChar(arg) != 1)
        return -1;

    /* input must be a row vector */
    if (mxGetM(arg)!=1) 
        return -1;
    
    /* get the length of the input string */
    buflen = (mxGetM(arg) * mxGetN(arg)) + 1;
    if (len) { *len = buflen; }

    /* copy the string data from prhs[0] into a C string input_ buf.    */
    *str = mxArrayToString(arg);
    
    if (*str == NULL) 
        return -1;
    
    return (0);
}

int get_bvgraph_args(const mxArray* smem, const mxArray* gmem, const mxArray* offsetmem, bvgraph *g)
{
    // check to make sure all the classes are valid
    if (mxGetClassID(smem) != mxUINT8_CLASS) {
        mexErrMsgIdAndTxt("bvgfun:invalidParameter", "smem is not a memory structure");
    }
    
    if (mxGetClassID(gmem) != mxUINT8_CLASS) {
        mexErrMsgIdAndTxt("bvgfun:invalidParameter", "gmem is not a graph file structure");
    }
    
    if (mxGetClassID(offsetmem) != mxUINT64_CLASS) {
        mexErrMsgIdAndTxt("bvgfun:invalidParameter", "offsetmem is not a valid offset structure");
    }
    
    if (mxGetNumberOfElements(smem) != sizeof(bvgraph)) {
        mexErrMsgIdAndTxt("bvgfun:invalidParameter", "smem is the wrong size (%i but should be %i)", 
            mxGetNumberOfElements(smem), sizeof(bvgraph));
    }
    
    // initialize the bvgraph
    memcpy(g, mxGetData(smem), sizeof(bvgraph));
    
    if (mxGetNumberOfElements(gmem) != g->memory_size) {
        mexErrMsgIdAndTxt("bvgfun:invalidParameter", "gmem is the wrong size (%i but should be %i)", 
            mxGetNumberOfElements(gmem), g->memory_size);
    }
    
    g->memory = mxGetData(gmem);
    
    if (g->offset_step > 0) 
    {
        if (mxGetNumberOfElements(offsetmem) != g->n) {
            mexErrMsgIdAndTxt("bvgfun:invalidParameter", "offsetmem is the wrong size (%i but should be %i)", 
                mxGetNumberOfElements(offsetmem), g->n);
        }
        g->offsets = mxGetData(offsetmem);
    }
    else {
        g->offsets = NULL;
    }
}

void mult_bvgraph(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
{
    bvgraph g;
    const mxArray* varg;
    int rval;
    
    if (nrhs != 5) {
        mexErrMsgIdAndTxt("bvgfun:invalidParameter",
            "incorrect number of parameters for mult operation");
    }
    
    varg = prhs[4];
    
    get_bvgraph_args(prhs[1], prhs[2], prhs[3], &g);
    if (!mxIsDouble(varg)) {
        mexErrMsgIdAndTxt("bvgfun:invalidParameter","the vector x is not a double");
    }
    if (!(mxGetM(varg) == g.n && mxGetN(varg) == 1)) {
        mexErrMsgIdAndTxt("bvgfun:invalidParameter",
            "the vector x has the wrong size (%i,%i but should be %i,%i)",
            mxGetM(varg), mxGetN(varg), g.n, 1);
    }
    
    
    plhs[0] = mxCreateDoubleMatrix(g.n, 1, mxREAL);
    if ((rval = bvgraph_mult(&g, mxGetPr(varg), mxGetPr(plhs[0]))) != 0) {
        mexErrMsgIdAndTxt("bvgfun:error",
            "libbvg reported error: %s", bvgraph_error_string(rval));
    }
}

void tmult_bvgraph(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
{
    bvgraph g;
    const mxArray* varg;
    int rval;
    
    if (nrhs != 5) {
        mexErrMsgIdAndTxt("bvgfun:invalidParameter",
            "incorrect number of parameters for transpose mult operation");
    }
    
    varg = prhs[4];
    
    get_bvgraph_args(prhs[1], prhs[2], prhs[3], &g);
    if (!mxIsDouble(varg)) {
        mexErrMsgIdAndTxt("bvgfun:invalidParameter","the vector x is not a double");
    }
    if (!(mxGetM(varg) == g.n && mxGetN(varg) == 1)) {
        mexErrMsgIdAndTxt("bvgfun:invalidParameter",
            "the vector x has the wrong size (%i,%i but should be %i,%i)",
            mxGetM(varg), mxGetN(varg), g.n, 1);
    }
    
    
    plhs[0] = mxCreateDoubleMatrix(g.n, 1, mxREAL);
    if ((rval = bvgraph_transmult(&g, mxGetPr(varg), mxGetPr(plhs[0]))) != 0) {
        mexErrMsgIdAndTxt("bvgfun:error",
            "libbvg reported error: %s", bvgraph_error_string(rval));
    }
}

void diag_bvgraph(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
{
    bvgraph g;
    int rval;
    
    if (nrhs != 4) {
        mexErrMsgIdAndTxt("bvgfun:invalidParameter",
            "incorrect number of parameters for diag operation");
    }
    
    get_bvgraph_args(prhs[1], prhs[2], prhs[3], &g);
    
    plhs[0] = mxCreateDoubleMatrix(g.n, 1, mxREAL);
    if ((rval = bvgraph_diag(&g, mxGetPr(plhs[0]))) != 0) {
        mexErrMsgIdAndTxt("bvgfun:error",
            "libbvg reported error: %s", bvgraph_error_string(rval));
    }
}

void sparse_bvgraph(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
{
    bvgraph g;
    int rval;
    
    if (nrhs != 4) {
        mexErrMsgIdAndTxt("bvgfun:invalidParameter",
            "incorrect number of parameters for sparse operation");
    }
    
    get_bvgraph_args(prhs[1], prhs[2], prhs[3], &g);

    plhs[0] = mxCreateSparseLogicalMatrix(g.n, g.n, g.m);
    
    #if MX_API_VER >= 0x07030000 && !defined(MX_COMPAT_32)
    rval = bvgraph_csr_large(&g, mxGetJc(plhs[0]), mxGetIr(plhs[0]));
    #else
    rval = bvgraph_csr(&g, mxGetJc(plhs[0]), mxGetIr(plhs[0]));
    #endif /* LARGE_ARRAYS */
    
    if (rval != 0) {
        mexErrMsgIdAndTxt("bvgfun:error",
            "libbvg reported error: %s", bvgraph_error_string(rval));
    }
    
    /* now set all the memory to 1 for all the logicals */
    memset(mxGetLogicals(plhs[0]), 1, sizeof(mxLogical)*g.m);
}

void substochastic_mult_bvgraph(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[], int tflag)
{
    bvgraph g;
    const mxArray* varg;
    int rval;
    
    if (nrhs != 5) {
        mexErrMsgIdAndTxt("bvgfun:invalidParameter",
            "incorrect number of parameters for substochastic_mult operation");
    }
    
    varg = prhs[4];
    
    get_bvgraph_args(prhs[1], prhs[2], prhs[3], &g);
    if (!mxIsDouble(varg)) {
        mexErrMsgIdAndTxt("bvgfun:invalidParameter","the vector x is not a double");
    }
    if (!(mxGetM(varg) == g.n && mxGetN(varg) == 1)) {
        mexErrMsgIdAndTxt("bvgfun:invalidParameter",
            "the vector x has the wrong size (%i,%i but should be %i,%i)",
            mxGetM(varg), mxGetN(varg), g.n, 1);
    }
    
    plhs[0] = mxCreateDoubleMatrix(g.n, 1, mxREAL);
    if (tflag == 0) {
        rval = bvgraph_substochastic_mult(&g, mxGetPr(varg), mxGetPr(plhs[0]));
    } else {
        rval = bvgraph_substochastic_transmult(&g, mxGetPr(varg), mxGetPr(plhs[0]));
    }
    
    if (rval != 0) {
        mexErrMsgIdAndTxt("bvgfun:error",
            "libbvg reported error: %s", bvgraph_error_string(rval));
    }
}

void sum_bvgraph(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[], int dim)
{
    bvgraph g;
    int rval;
    
    if (nrhs != 4) {
        mexErrMsgIdAndTxt("bvgfun:invalidParameter",
            "incorrect number of parameters for sum operation");
    }
    
    get_bvgraph_args(prhs[1], prhs[2], prhs[3], &g);
    
    plhs[0] = mxCreateDoubleMatrix(g.n, 1, mxREAL);
    if (dim == 0) { 
        /* compute the column-sums */
        rval = bvgraph_sum_col(&g, mxGetPr(plhs[0]));
    } else if (dim == 1) {
        /* compute the row-sums */
        rval = bvgraph_sum_row(&g, mxGetPr(plhs[0]));
    } else {
        mexErrMsgIdAndTxt("bvgfun:invalidParameter", "unknown sum dimension %i", dim);
    }
    
    if (rval != 0) {
        mexErrMsgIdAndTxt("bvgfun:error",
            "libbvg reported error: %s", bvgraph_error_string(rval));
    }
}

void load_bvgraph(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
{
    const mxArray *fnarg;
    const mxArray *osarg;
    int offset_step;
    const char *filename;
	mwSize filenamelen;
    int rval;
    bvgraph g;
    
    if (nrhs != 3) {
        mexErrMsgIdAndTxt("bvgfun:invalidParameter",
            "incorrect number of parameters for load operation");
    }
    
    fnarg = prhs[1];
    osarg = prhs[2];
    
    if (get_string_arg(fnarg, &filename, &filenamelen) != 0) {
        mexErrMsgIdAndTxt("bvgfun:invalidParameter",
            "filename could not be converted to a string");
    }
    filenamelen -= 1;
    
    if (mxGetNumberOfElements(osarg) != 1 || !mxIsDouble(osarg)) {
        mexErrMsgIdAndTxt("bvgfun:invalidParameter",
            "offset_step is not a scalar double");
    }
    
    offset_step = (int)mxGetScalar(osarg);
    
    if (offset_step == -1) {
        rval = bvgraph_load(&g, filename, filenamelen, offset_step);
        if (rval != 0) {
            mexErrMsgIdAndTxt("bvgfun:error",
                "libbvg reported error: %s", bvgraph_error_string(rval));
        }
        plhs[3] = mxCreateNumericMatrix(0, 1, mxUINT8_CLASS, mxREAL);
        plhs[4] = mxCreateNumericMatrix(0, 1, mxUINT64_CLASS, mxREAL);
    }
    else if (offset_step == 0) {
        // first load the graph to figure out how much memory it will take
        size_t gbuf;
        mxArray *gmem, *offsetmem;
        
        rval = bvgraph_load(&g, filename, filenamelen, -1);
        if (rval != 0) {
            mexErrMsgIdAndTxt("bvgfun:error",
                "libbvg reported error (initial load): %s", bvgraph_error_string(rval));
        }
        rval = bvgraph_required_memory(&g, offset_step, &gbuf, NULL);
        if (rval != 0) {
            mexErrMsgIdAndTxt("bvgfun:error",
                "libbvg reported error (memory): %s", bvgraph_error_string(rval));
        }
        bvgraph_close(&g);

        gmem = plhs[3] = mxCreateNumericMatrix(gbuf, 1, mxUINT8_CLASS, mxREAL);
        offsetmem = plhs[4] = mxCreateNumericMatrix(0, 1, mxUINT64_CLASS, mxREAL);
        
        rval = bvgraph_load_external(&g, filename, filenamelen, offset_step,
            mxGetData(gmem), mxGetNumberOfElements(gmem),
            mxGetData(offsetmem), mxGetNumberOfElements(offsetmem));
        if (rval != 0) {
            mexErrMsgIdAndTxt("bvgfun:error",
                "libbvg reported error (final load): %s", bvgraph_error_string(rval));
        }
    }
    else {
        mexErrMsgIdAndTxt("bvgfun:invalidParameter",
            "offset_step (%i) is not supported!", offset_step);
    }
    
    plhs[2] = mxCreateNumericMatrix(sizeof(bvgraph), 1, mxUINT8_CLASS, mxREAL);
    memcpy(mxGetData(plhs[2]), &g, sizeof(bvgraph));
    
    plhs[0] = mxCreateDoubleScalar((double)g.n);
    plhs[1] = mxCreateDoubleScalar((double)g.m);
}

void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
{
    const char *op;
    mwSize oplen;
    
    if (nrhs < 3) {
        mexErrMsgIdAndTxt("bvgfun:invalidParameter", "insufficient parameters");
    }
    
    
    if (get_string_arg(prhs[0], &op, &oplen) != 0) {
        mexErrMsgIdAndTxt("bvgfun:invalidParameter", "op could not be converted to a string");
    }
    
    if (strncmp(op, "load", oplen) == 0) {
        load_bvgraph(nlhs, plhs, nrhs, prhs);
    } else if (strncmp(op, "mult", oplen) == 0) {
        mult_bvgraph(nlhs, plhs, nrhs, prhs);
    } else if (strncmp(op, "tmult", oplen) == 0) {
        tmult_bvgraph(nlhs, plhs, nrhs, prhs);
    } else if (strncmp(op, "diag", oplen) == 0) {
        diag_bvgraph(nlhs, plhs, nrhs, prhs);
    } else if (strncmp(op, "sparse", oplen) == 0) {
        sparse_bvgraph(nlhs, plhs, nrhs, prhs);
    } else if (strncmp(op, "substochastic_mult", oplen) == 0) {
        substochastic_mult_bvgraph(nlhs, plhs, nrhs, prhs, 0);    
    } else if (strncmp(op, "substochastic_tmult", oplen) == 0) {
        substochastic_mult_bvgraph(nlhs, plhs, nrhs, prhs, 1);
    } else if (strncmp(op, "sum1", oplen) == 0) {
        sum_bvgraph(nlhs, plhs, nrhs, prhs, 0);
    } else if (strncmp(op, "sum2", oplen) == 0) {
        sum_bvgraph(nlhs, plhs, nrhs, prhs, 1);
    } else {
        mexErrMsgIdAndTxt("bvgfun:invalidParameter", "unknown operation");
    }
    
    
    
}
    
