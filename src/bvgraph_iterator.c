/*
 * David Gleich
 * Copyright, Stanford University, 2007
 * 18 May 2007
 */

/**
 * @file bvgraph_iterator.c
 * Implement the set of routines to work with the bvgraph iterator
 */

/** History
 *
 * 2007-06-10
 * Changed iterator to allocate block,len,left,buf1,buf2 only once
 *   per iterator instead of once per next call.  This improved the 
 *   iteration speed by almost 3x for cnr-2000.  Added corresponding
 *   allocation and deallocation to bvgraph_nonzero_iterator and 
 *   bvgraph_iterator_free.
 */

#include "bvgraph_internal.h"

/**
 * Prototypes for internal functions
 */

inline int nat2int(const int x) { return x % 2 == 0 ? x >> 1 : -( ( x + 1 ) >> 1 ); }

inline int read_coded(bitfile *bf, enum bvgraph_compression_flag_tag c) 
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
inline int read_offset(bvgraph *g, bitfile *bf) { return read_coded(bf, g->offset_coding); }
inline int read_outdegree(bvgraph *g, bitfile *bf) { return read_coded(bf, g->outdegree_coding); }
inline int read_residual(bvgraph *g, bitfile *bf) 
{ 
    if (g->residual_coding == BVGRAPH_FLAG_ZETA) {
        return bitfile_read_zeta(bf,g->zeta_k);
    } else {
        return read_coded(bf, g->residual_coding); 
    }
}
inline int read_reference(bvgraph *g, bitfile *bf) { return read_coded(bf, g->reference_coding); }
inline int read_block(bvgraph *g, bitfile *bf) { return read_coded(bf, g->block_coding); }
inline int read_block_count(bvgraph *g, bitfile *bf) { return read_coded(bf, g->block_count_coding); }

/**
 * Create a non-zero iterator for the bvgraph.  The non-zero iterator is 
 * a new object like structure that iterates over the successors of each node
 * in the graph.  There can be many nonzero iterators and each nonzero iterator
 * is independent.
 *
 * The nonzero iterator requires persistent memory to work and is useful
 * for a single iteration over the file.  Each new iteration requires a new
 * nonzero iterator at present.
 */
int bvgraph_nonzero_iterator(bvgraph* g, bvgraph_iterator *i)
{
    int rval = 0;
    int outd_alloc = 10;
    int windcount = 0;

    // check and see if we know something better about the maximum outdegree
    if (g->max_outd != 0) {
        // if we set this variable, then we will avoid all
        // reallocations as we pass through the file.  I'm not 
        // sure if that is an issue, but it may be.
        outd_alloc = g->max_outd;
        i->max_outd = g->max_outd;
    }
    else {
        i->max_outd = 0; 
    }

    i->g = g;
    i->curr = -1;
    i->curr_outd = -1;
    i->cyclic_buffer_size = i->g->window_size+1;

    if (g->offset_step == -1) {
        char *graphfilename = strappend(g->filename, g->filenamelen, ".graph", 6);
        FILE *f = fopen(graphfilename, "rb");
        free(graphfilename);
        if (!f) { return bvgraph_call_io_error; }

        rval = bitfile_open(f,&i->bf);
        if (rval) { return rval; }
    } else if (g->offset_step == 0) {
        rval = bitfile_map(g->memory, g->memory_size, &i->bf);
    } else {
        return bvgraph_call_unsupported;
    }

    // beyond this point, the bitfile was successfully allocated, so we must 
    // deallocate it if we exit.

    i->outd_cache = malloc(sizeof(int)*i->cyclic_buffer_size);
    if (i->outd_cache) {
        i->window = malloc(sizeof(bvgraph_int_vector)*i->cyclic_buffer_size);
        if (i->window) {
            rval = int_vector_create(&i->successors, outd_alloc);
            if (rval == 0) {
                for (windcount = 0; windcount < i->cyclic_buffer_size; windcount++) {
                    rval = int_vector_create(&i->window[windcount], outd_alloc);
                    if (rval != 0) { break; }
                }

                // 
                // this statement here indicates we should return
                // with success and a correctly allocated 
                // interator
                //
                
                rval = int_vector_create(&i->block, outd_alloc);
                rval = int_vector_create(&i->left, outd_alloc);
                rval = int_vector_create(&i->len, outd_alloc);
                rval = int_vector_create(&i->buf1, outd_alloc);
                rval = int_vector_create(&i->buf2, outd_alloc);

                if (rval == 0) { 
                    rval = bvgraph_iterator_next(i);
                    if (rval == 0) {
                        return (0); 
                    }
                    // we failed to fetch the first set of
                    // indices, this indicates that we 
                    // failed in creation and should free
                    // ourselves
                }

                // in this case, we have to free everything allocated
                while (windcount >= 0) {
                    int_vector_free(&i->window[windcount]);
                    windcount--;
                }

                int_vector_free(&i->successors);
            }
            free(i->window);
        }
        else { rval = bvgraph_call_out_of_memory; }

        free(i->outd_cache);
    } 
    else { rval = bvgraph_call_out_of_memory; }
    bitfile_close(&i->bf);
  
    return rval;
}

/**
 * This routine provides access to the successors array stored inside
 * the iterator, so the successor array will be invalidated after 
 * a call to iterator_next.
 */
int bvgraph_iterator_outedges(bvgraph_iterator* i, int** start, unsigned int* len)
{
    if (start) { *start = i->successors.a; }
    if (len) { *len = i->curr_outd; }

    return (0);
}

/**
 * Merege two sorted arrays into a single sorted array.
 *
 * @param a1 the first array
 * @param a1len the length of the first array
 * @param a2 the second array
 * @param a2len the length of the second array
 * @param out the final array
 * @param outlen the length of the output array
 */
int merge_int_arrays(const int* a1, size_t a1len, const int* a2, size_t a2len,
                     int *out, size_t outlen)
{
    size_t a1i=0, a2i=0, oi=0;
    // make sure we don't have to worry about having enough space
    if (outlen < a1len + a2len) {
        return bvgraph_call_unsupported; 
    }
    while (a1i < a1len && a2i < a2len) {
        out[oi++] = a1[a1i] < a2[a2i] ? a1[a1i++] : a2[a2i++];
    }
    if (a1i < a1len) {
        memcpy(&out[oi], &a1[a1i], (a1len - a1i)*sizeof(int));
    } else {
        memcpy(&out[oi], &a2[a2i], (a2len - a2i)*sizeof(int));
    }
    return (0);
}

/**
 * Load the next set of successors into the i->successors array.  This
 * routine invalidates the current set of successors.
 *
 * TODO: Optimize memory usage in this routine.
 */
int bvgraph_iterator_next(bvgraph_iterator* iter)
{
    // 
    const int x = ++(iter->curr);
    int ref, ref_index;
    int i, extra_count, block_count = 0;

    // TODO: make these static arrays for the iterator
    // bvgraph_int_vector block, left, len, buf1, buf2;

    bvgraph *g = iter->g;
    bitfile *bf = &iter->bf;

    int d, copied, total, interval_count;
    int buf1_index, buf2_index;

    // make sure the iterator is still valid
    if (!bvgraph_iterator_valid(iter)) {
        return bvgraph_call_unsupported;
    }

    d = iter->outd_cache[x%iter->cyclic_buffer_size]=read_outdegree(g, bf);
    iter->curr_outd = d;

    if (d > iter->max_outd) { iter->max_outd = d; }

    if (d == 0) {
        // nothing to do!
        return (0);
    }

    // allocate the structures
    // TODO perform error checking here
    /*int_vector_create(&block, 10);
    int_vector_create(&left, 10);
    int_vector_create(&len, 10);

    // allocate a sufficient buffer for the output
    int_vector_ensure_size(&iter->successors, d);
    int_vector_create(&buf1, d);
    int_vector_create(&buf2, d);*/

    int_vector_ensure_size(&iter->successors, d);
    int_vector_ensure_size(&iter->buf1, d);
    int_vector_ensure_size(&iter->buf2, d);

#ifdef MAX_DEBUG
    fprintf(stderr, "** begin successors\ncurr = %i\n", iter->curr);
#endif 
            
    // we read the reference only if the actual window size is larger than one 
    // (i.e., the one specified by the user is larger than 0).
    if ( g->window_size > 0 ) {
        ref = read_reference(g, bf);
        // TODO: check success
    }
            
    ref_index = (x - ref + iter->cyclic_buffer_size) % iter->cyclic_buffer_size;
    // TODO: check for valid reference
    if (ref > 0)
    {
        if ( (block_count = read_block_count(g, bf)) != 0 ) {
            // TODO: test success
            int_vector_ensure_size(&iter->block, block_count);
        }

#ifdef MAX_DEBUG
    fprintf(stderr, "block_count = %i\n", block_count);
#endif 
        // the number of successors copied, and the total number of successors specified
        // in some copy
        copied = 0; 
        total = 0;

        for (i = 0; i < block_count; i++) {
            iter->block.a[i] = read_block(g, bf) + (i == 0 ? 0 : 1);
            // TODO: test success
            total += iter->block.a[i];
            if (i % 2 == 0) {
                copied += iter->block.a[i];
            }
        }
        if (block_count%2 == 0) {
            copied += (iter->outd_cache[ref_index] - total);
        }
        // TODO: error on copied > d
        extra_count = d - copied;
    }
    else {
        extra_count = d;
    }
            
    interval_count = 0;
    if (extra_count > 0)
    {
        if (g->min_interval_length != 0 && (interval_count = bitfile_read_gamma(bf)) != 0) 
        {
            int prev = 0;

            // TODO: test success
            int_vector_ensure_size(&iter->left, interval_count);
            int_vector_ensure_size(&iter->len, interval_count);
            
            // now read the intervals
            iter->left.a[0] = prev = nat2int(bitfile_read_gamma(bf)) + x;
            iter->len.a[0] = bitfile_read_gamma(bf) + g->min_interval_length;
            
            prev += iter->len.a[0];
            extra_count -= iter->len.a[0];
            
            for (i=1; i < interval_count; i++) {
                iter->left.a[i] = prev = bitfile_read_gamma(bf) + prev + 1;
                iter->len.a[i] = bitfile_read_gamma(bf) + g->min_interval_length;
                prev += iter->len.a[i];
                extra_count -= iter->len.a[i];
            }
        }
    }
        
    buf1_index = 0;
    buf2_index = 0;

#ifdef MAX_DEBUG
    fprintf("extra_count = %i\ninterval_count = %i\nref = %i\n", extra_count, interval_count, ref);
#endif 
    // read the residuals into a buffer
    {
        int prev = -1;
        int residual_count = extra_count;
        while (residual_count > 0) {
            residual_count--;
            if (prev == -1) { iter->buf1.a[buf1_index++] = prev = x + nat2int(read_residual(g, bf)); }
            else { iter->buf1.a[buf1_index++] = prev = read_residual(g, bf) + prev + 1; }
        }
        // std::cout << "residuals: buf1" << std::endl;
        // std::copy(buffer1.begin(),buffer1.begin()+buf1_index,std::ostream_iterator<int>(std::cout, "\n"));
    }
    // std::cout << "buf1_index = " << buf1_index << std::endl;
                
    if (interval_count == 0)
    {
        // don't do anything
    }
    else
    {
        // copy the extra interval data
        for (i = 0; i < interval_count; i++)
        {
            int j, cur_left = iter->left.a[i];
            for (j = 0; j < iter->len.a[i]; j++) {
                iter->buf2.a[buf2_index++] = cur_left + j;
            }
        }
        
        // std::cout << "sequences: buf2" << std::endl;
        // std::copy(buffer2.begin(),buffer2.begin()+buf2_index,std::ostream_iterator<int>(std::cout, "\n"));
        
        if (extra_count > 0)
        {
            // merge buf1, buf2 into arcs
            merge_int_arrays(iter->buf1.a, buf1_index, iter->buf2.a, buf2_index, 
                iter->successors.a, iter->successors.elements);
            
            // now copy arcs back to buffer1, and free buffer2
            buf1_index = buf1_index + buf2_index;
            buf2_index = 0;           
            memcpy(iter->buf1.a, iter->successors.a, buf1_index*sizeof(int));
        }
        else
        {
            memcpy(iter->buf1.a, iter->buf2.a, buf2_index*sizeof(int));
            buf1_index = buf2_index;
            buf2_index = 0;
        }
    }
            
    if (ref <= 0)
    {
        // don't do anything except copy
        // the data to arcs
        if (interval_count == 0 || extra_count == 0) {
            memcpy(iter->successors.a, iter->buf1.a, sizeof(int)*buf1_index);
        }
    }
    else
    {          
        // TODO clean this code up          
        // copy the information from the masked iterator
        
        int mask_index = 0;
        // this variable is intended to shadow the vector len
        int len = 0;
        for (i=0; i < iter->outd_cache[ref_index]; )
        {
            if (len <= 0)
            {
                if (block_count == mask_index) 
                {
                    if (block_count % 2 == 0) {
                        len = iter->outd_cache[ref_index] - i;
                    }
                    else {
                        break;
                    }
                }
                else {
                    if (mask_index % 2 == 0) { len = iter->block.a[mask_index++]; }
                    else { i += iter->block.a[mask_index++]; continue; }
                }
                
                // in the case that length is 0, we continue.
                if (len == 0) { continue; }
            }
            iter->buf2.a[buf2_index++] = iter->window[ref_index].a[i];
            len--;
            i++;
        }
        
        // std::cout << "masked" << std::endl;
        // std::copy(buffer2.begin(),buffer2.begin()+buf2_index,std::ostream_iterator<int>(std::cout, "\n"));
     
        merge_int_arrays(iter->buf1.a, buf1_index, iter->buf2.a, buf2_index, 
            iter->successors.a, iter->successors.elements);

        buf1_index = buf1_index + buf2_index;
        buf2_index = 0;
    }

    // update the window
    {
        int curr_index = iter->curr % iter->cyclic_buffer_size;
        int_vector_ensure_size(&iter->window[curr_index], d);
        
        // unwrap the buffered output
        memcpy(iter->window[curr_index].a, iter->successors.a, sizeof(int)*d);
    }

    /*int_vector_free(&block);
    int_vector_free(&left);
    int_vector_free(&len);
    int_vector_free(&buf1);
    int_vector_free(&buf2);*/

    return (0);
}

/**
 * @param i the bvgraph iterator
 * @return 1 if the iterator is still valid, 0 otherwise
 */
int bvgraph_iterator_valid(bvgraph_iterator* i)
{
    if (i->g && i->curr < i->g->n) { return 1; }
    return (0);
}

int bvgraph_iterator_free(bvgraph_iterator *iter)
{
    int i;
    if (iter->g && iter->curr == iter->g->n+1) { 
        iter->g->max_outd = iter->max_outd;
    }
    bitfile_close(&iter->bf);
    free(iter->outd_cache);
    int_vector_free(&iter->successors);
    for (i=0; i < iter->cyclic_buffer_size; i++) {
        int_vector_free(&iter->window[i]);
    }
    free(iter->window);
    int_vector_free(&iter->block);
    int_vector_free(&iter->left);
    int_vector_free(&iter->len);
    int_vector_free(&iter->buf1);
    int_vector_free(&iter->buf2);
    iter->g = NULL;
    return (0);
}

