/*
 * David Gleich
 * Copyright, Stanford University, 2008
 * 10 March 2008
 */

/**
 * @file bvgraph_inline_io.h
 * Implement all the routines to read pieces of a bvgraph file.
 * @author David Gleich
 * @date 10 March 2008
 * @brief implementations all the routines
 *
 * @version
 *
 * 2008-03-10: Coding started, ported codes from bvgraph_iterator.c
 */

#include "bvgraph_internal.h"

static inline int64_t nat2int(const int64_t x) { return x % 2 == 0 ? x >> 1 : -( ( x + 1 ) >> 1 ); }

static inline int64_t read_coded(bitfile *bf, enum bvgraph_compression_flag_tag c) 
{
    switch (c) {
        case BVGRAPH_FLAG_DELTA: 
        case BVGRAPH_FLAG_GOLOMB:
        case BVGRAPH_FLAG_SKEWED_GOLOMB:
        case BVGRAPH_FLAG_ARITH:
        case BVGRAPH_FLAG_INTERP:
        case BVGRAPH_FLAG_ZETA:
            return (bvgraph_call_unsupported);

        case BVGRAPH_FLAG_GAMMA:
            return bitfile_read_gamma(bf);
            break;

        case BVGRAPH_FLAG_UNARY:
            return bitfile_read_unary(bf);
            break;

        case BVGRAPH_FLAG_NIBBLE:
            return bitfile_read_nibble(bf);
            break;
    }

    return (bvgraph_call_unsupported);
}
static inline int64_t read_offset(bvgraph *g, bitfile *bf) { return read_coded(bf, g->offset_coding); }
static inline int64_t read_outdegree(bvgraph *g, bitfile *bf) { return read_coded(bf, g->outdegree_coding); }
static inline int64_t read_residual(bvgraph *g, bitfile *bf) 
{ 
    if (g->residual_coding == BVGRAPH_FLAG_ZETA) {
        return bitfile_read_zeta(bf,g->zeta_k);
    } else {
        return read_coded(bf, g->residual_coding); 
    }
}
static inline int64_t read_reference(bvgraph *g, bitfile *bf) { return read_coded(bf, g->reference_coding); }
static inline int64_t read_block(bvgraph *g, bitfile *bf) { return read_coded(bf, g->block_coding); }
static inline int64_t read_block_count(bvgraph *g, bitfile *bf) { return read_coded(bf, g->block_count_coding); }

/** Skips outdegrees from the given stream. 
 *
 * @param g the graph-structure
 * @param bf a graph-file input bit stream.
 * @param count the number of outdegrees to skip.
 */
static inline int skip_outdegrees(bvgraph *g, bitfile *bf, const int count) 
{
    switch (g->outdegree_coding) {
        case BVGRAPH_FLAG_GAMMA: return bitfile_skip_gammas(bf,count);
        case BVGRAPH_FLAG_DELTA: return bitfile_skip_deltas(bf,count);
        default: return bvgraph_call_unsupported;
    }
}

//inline int fill_node_buffers(bvgraph *g, bitfile *bf,
