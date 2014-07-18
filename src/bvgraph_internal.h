#ifndef LIBBVG_BVGRAPH_INTERNAL_H
#define LIBBVG_BVGRAPH_INTERNAL_H

/*
 * David Gleich
 * Copyright, Stanford University, 2007
 * 17 May 2007
 */

/**
 * @file bvgraph_internal.h
 * Prototype a series of internal routines for the bvgraph library
 *
 * @version
 * 
 *  2008-05-08: Added int_vector_create_copy
 */ 

#include "bvgraph.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef unsigned int uint;

extern char return_error_string[];

extern int ftestnewline(FILE *f);
extern int64_t atoin(const char* str, uint len);
extern char* strappend(const char* str, uint len, const char* str2, uint len2);
const char* strchrn(const char* str, uint len, int c);
extern void fnextline(FILE *f);
extern void fskipchars(FILE *f, const char *schars, uint scharslen);
extern int fsize(const char *filename, unsigned long long *s);

extern int parse_compression_flags(bvgraph* g, const char* flagstr, uint len);
extern char* parse_property_key(FILE *f, uint maxproplen);
extern char* parse_property_value(FILE *f, uint maxvallen);
extern int parse_properties(bvgraph* g);

extern int int_vector_create(bvgraph_int_vector* v, uint64_t n);
extern int int_vector_create_copy(bvgraph_int_vector* u, bvgraph_int_vector *v);
extern int int_vector_ensure_size(bvgraph_int_vector *v, uint64_t n);
extern int int_vector_free(bvgraph_int_vector* v);

//
// bvgraph_io routines
//
/*inline int nat2int(const int x);
inline int read_coded(bitfile *bf, enum bvgraph_compression_flag_tag c);
inline int read_offset(bvgraph *g, bitfile *bf);
inline int read_outdegree(bvgraph *g, bitfile *bf);
inline int read_residual(bvgraph *g, bitfile *bf);
inline int read_reference(bvgraph *g, bitfile *bf);
inline int read_block(bvgraph *g, bitfile *bf);
inline int read_block_count(bvgraph *g, bitfile *bf);*/

// disable all of the unsafe operation warnings
#ifdef _MSC_VER
#define inline __inline
#if _MSC_VER >= 1400
#pragma warning ( push )
#pragma warning ( disable: 4996 )
#endif /* _MSC_VER >= 1400 */
#endif /* _MSC_VER */


#endif /* #ifndef LIBBVG_BVGRAPH_INTERNAL_H */

