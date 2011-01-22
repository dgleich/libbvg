#ifndef LIBBVG_BVGRAPH_H
#define LIBBVG_BVGRAPH_H

/*
 * David Gleich
 * Copyright, Stanford University, 2007
 * 17 May 2007
 */

/**
 * @file bvgraph.h
 * This file is the main public header for LIBBVG
 */

/** History
 *
 * 2007-06-10: Added block,len,left,buf1,buf2 fields to the iterator types.
 *
 * 2008-03-10: Defined long long as bvg_long_int for a 64-bit type
 *             Added more comments.
 *             Fixed lines over 80 characters long.
 *             Added prototypes for random access iterator.

 * 2008-05-08: Added iterator_copy
 * 2008-05-09: Added parallel iterators
 *             Added BVGRAPH_VERBOSE macro
 */

#include "bitfile.h"

//#define MAX_DEBUG

#ifndef BVGRAPH_VERBOSE
#define BVGRAPH_VERBOSE 0
#endif

#define BVGRAPH_MAX_FILENAME_SIZE 1024

/** Define a 64-bit type 
 */
typedef long long bvg_long_int;

enum bvgraph_compression_flag_tag {
    BVGRAPH_FLAG_DELTA = 1,
    BVGRAPH_FLAG_GAMMA = 2,
    BVGRAPH_FLAG_GOLOMB = 3,
    BVGRAPH_FLAG_SKEWED_GOLOMB = 4,
    BVGRAPH_FLAG_ARITH = 5,
    BVGRAPH_FLAG_INTERP = 6,
    BVGRAPH_FLAG_UNARY = 7,
    BVGRAPH_FLAG_ZETA = 8,
    BVGRAPH_FLAG_NIBBLE = 9,
};

typedef enum bvgraph_compression_flag_tag bvgraph_compression_flag;

/** The main bvgraph structure represents a Boldi-Vigna graph file in memory.
 * 
 * You should use this structure through its alias bvgraph instead.
 */
struct bvgraph_tag {
    // graph name information
    char filename[BVGRAPH_MAX_FILENAME_SIZE];
    unsigned int filenamelen;

    // graph size information
    int n;
    bvg_long_int m;
    int max_outd;   // max_outd = 0 if it is unknown

    // graph compression parameters
    int max_ref_count;
    int window_size;
    int min_interval_length;
    int zeta_k;

    enum bvgraph_compression_flag_tag outdegree_coding;
    enum bvgraph_compression_flag_tag block_coding;
    enum bvgraph_compression_flag_tag residual_coding;
    enum bvgraph_compression_flag_tag reference_coding;
    enum bvgraph_compression_flag_tag block_count_coding;
    enum bvgraph_compression_flag_tag offset_coding;

    // graph load information
    int offset_step;

    unsigned char* memory;
    size_t memory_size;
    int memory_external;

    unsigned long long* offsets;
    int offsets_external;
};

/** A simple integer vector type used in the iterator.
 * 
 * This type is internal, you should not use it.  I could have made the type
 * opaque and hidden it through internal linking and other such techniques.
 * Making it available, however, makes the code more transparent.  
 * (Okay, if you really must use it, just see how I use it, but no promises
 * that I won't change the interface.)
 */
struct bvgraph_int_vector_tag {
    unsigned int elements;
    int* a;
};

/** A sequential access iterator for the bvgraph.
 *
 * Use this type through its alias bvgraph_iterator.
 *
 * A bvgraph_iterator gives a sequential view of a bvgraph by iterating
 * over its edges.
 */
struct bvgraph_iterator_tag {
    // variables that will be maintained for a public interface
    int curr;
    struct bvgraph_tag* g;
    bitfile bf;

    // implementation dependent variables
    int cyclic_buffer_size;
    int* outd_cache;
    struct bvgraph_int_vector_tag* window;

    struct bvgraph_int_vector_tag successors; 
    int curr_outd;

    // variables used inside the next function
    int max_outd;
    struct bvgraph_int_vector_tag block, left, len, buf1, buf2;
};

/** A random access iterator for the bvgraph.
 *
 * Use this type through its alias bvgraph_iterator.
 *
 * A bvgraph_iterator gives a sequential view of a bvgraph by iterating
 * over its edges.
 */
struct bvgraph_random_iterator_tag {
    // variables that will be maintained for a public interface
    int curr;
    struct bvgraph_tag* g;
    

    // implementation dependent variables

    // maintain two bitfiles, one for the the outdegrees, one for successors
    bitfile bf;
    bitfile outd_bf;

    int offset_step;
    bvg_long_int *offsets;

    // a cache for outdegrees
    int* outd_cache;
    // the endpoint of the outdegree cache
    int outd_cache_end;
    // a cache for offests
    bvg_long_int* offset_cache;
    // the end of the offset cache
    int offset_cache_end;
    // the start of both caches
    int cache_start;            

    int cyclic_buffer_size;
    struct bvgraph_int_vector_tag* window;

    struct bvgraph_int_vector_tag successors; 
    int curr_outd;

    // variables used inside the next function
    int max_outd;
    struct bvgraph_int_vector_tag block, left, len, buf1, buf2;
};


struct bvgraph_parallel_iterators_tag {
    int niters;
    int *nsteps;
    struct bvgraph_iterator_tag *iters;
    struct bvgraph_tag *g;
};

typedef struct bvgraph_tag bvgraph;
typedef struct bvgraph_iterator_tag bvgraph_iterator;
typedef struct bvgraph_random_iterator_tag bvgraph_random_iterator;
typedef struct bvgraph_int_vector_tag bvgraph_int_vector;
typedef struct bvgraph_parallel_iterators_tag bvgraph_parallel_iterators;

// define all the error codes
extern const int bvgraph_call_out_of_memory;
extern const int bvgraph_call_io_error;
extern const int bvgraph_call_unsupported;

extern const int bvgraph_load_error_filename_too_long;
extern const int bvgraph_load_error_buffer_too_small;

extern const int bvgraph_property_file_error;
extern const int bvgraph_property_file_compression_flag_error;
extern const int bvgraph_unsupported_version;

extern const int bvgraph_vertex_out_of_range;
extern const int bvgraph_requires_offsets;
extern const int bvgraph_unsupported_coding;

int bvgraph_load(bvgraph* g, const char *filename, unsigned int filenamelen,
                 int offset_step);

int bvgraph_load_external(bvgraph *g,
                          const char *filename, unsigned int filenamelen, 
                          int offset_step,
                          unsigned char *gmemory, size_t gmemsize,
                          unsigned long long* offsets, int offsetssize);

int bvgraph_close(bvgraph* g);
int bvgraph_nonzero_iterator(bvgraph* g, bvgraph_iterator *i);
int bvgraph_random_access_iterator(bvgraph* g, bvgraph_random_iterator *ri);

int bvgraph_outdegree(bvgraph *g, int x, unsigned int *d);
int bvgraph_successors(bvgraph *g, int x);

int bvgraph_iterator_outedges(bvgraph_iterator* i, 
                              int** start, unsigned int* len);
int bvgraph_iterator_next(bvgraph_iterator* i);
int bvgraph_iterator_valid(bvgraph_iterator* i);
int bvgraph_iterator_free(bvgraph_iterator *i);

int bvgraph_random_outdegree(bvgraph_random_iterator *ri, 
                             int x, unsigned int *d);
int bvgraph_random_successors(bvgraph_random_iterator *ri, 
                             int x, int** start, unsigned int *len);
int bvgraph_random_free(bvgraph_random_iterator *ri);

int bvgraph_iterator_copy(bvgraph_iterator *i, bvgraph_iterator *j);

int bvgraph_parallel_iterators_create(bvgraph *g, 
        bvgraph_parallel_iterators *pits, int niters, int wnode, int wedge);
int bvgraph_parallel_iterator(bvgraph_parallel_iterators *pits, int i,
        bvgraph_iterator *iter, int *nsteps);
int bvgraph_parallel_iterators_free(bvgraph_parallel_iterators *pits);


int bvgraph_required_memory(bvgraph *g, 
                            int offset_step, size_t *gbuf, size_t *offsetbuf);

const char* bvgraph_error_string(int error);

#endif // LIBBVG_BVGRAPH_H
