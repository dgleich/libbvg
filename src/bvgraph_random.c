/*
 * David Gleich
 * Copyright, Stanford University, 2008
 * 10 March 2008
 */

/**
 * @file bvgraph_random.c
 * Implement the set of routines to work with the bvgraph 
 * with random access, but only when offset_step = 1
 * @author David Gleich
 * @date 10 March 2008
 * @brief implementation of routines for bvgraph with random access
 *
 * @version
 *
 * 2008-03-10: Coding started
 */

#include "bvgraph_internal.h"
#include "bvgraph_inline_io.h"
#include <inttypes.h>

struct successor *CACHE = NULL;

/** Declare static methods for this iterator
 */

/** Skip the successors of a node (after the outdegree part).
 *
 * <P>This method must be called with <code>ibs</code> positioned exactly at the
 * beginning of the successor list of node <code>x</code>, excluding the outdegree,
 * which must be provided in <code>outd</code>.
 * 
 * <P><strong>Warning</strong>: This method duplicates unavoidably part of the
 * logic of {@link #successors(int, InputBitStream, int[][], int[], int[])}; the
 * two methods must remain tightly coupled.
 *
 * @param ri the random access iterator, with the bitfile positioned exactly
 *           at the start of the successor list
 * @param x the node for the successor list
 * @param outd the outdegree of node x
 * @return 0 on success
 */

/*
static int skip_node(bvgraph_random_iterator *ri,
                     int x, unsigned int d)
{
    return (bvgraph_call_unsupported);
}
*/

/** Positions the given input bit stream exactly before the successor list of the
 * given node, just after the outdegree, which is returned.  
 *
 * @param ri the random iterator
 * @param x the index of the node
 * @param[out] d the outdegree
 */
static int position_bvgraph(bvgraph_random_iterator *ri, int64_t x, uint64_t *d)
{
    if (x < 0 || x >= ri->g->n) {
        return (bvgraph_vertex_out_of_range);
    }

    if (ri->offset_step <= 0) {
        return bvgraph_requires_offsets;
    } else if (ri->offset_step == 1) {
        int rval = bitfile_position(&ri->bf, ri->g->offsets[x]);
        if (rval == 0) {
            *d = read_outdegree(ri->g, &ri->bf);
        }
        return rval;
    } else {
        return bvgraph_call_unsupported;
    }
}

/** Get the outdegree for a vertex in the graph.
 *
 * This operation just modifies the special outdegree specific bitfile
 * and not the successor bitfile.  
 * 
 * @param[in] ri a random access iterator for the graph
 * @param[in] i the index of the node (i in [0,g->n-1])
 * @param[out] d the node degree.
 * @return 0 on success
 */
int bvgraph_random_outdegree(bvgraph_random_iterator *ri, 
                             int64_t i, uint64_t *d)
{
    if (i<0 || i >= ri->g->n) {
        return (bvgraph_vertex_out_of_range);
    }
    
    // TODO: always add outd_cache search
    if (ri->offset_step <= 0) {
        return (bvgraph_requires_offsets);
    } else if (ri->offset_step == 1) {
        bitfile_position(&ri->outd_bf, ri->g->offsets[i]);
        *d = read_outdegree(ri->g, &ri->outd_bf);
        return (0);
    } else {
        // code for the case when offset_step > 1
        // check if its in the outd cache
        // if (i >= ri->cache_start && i < ri->outd_cache_end) {
        //     *d = ri->outd_cache[i-ri->cache_start];
        //     return (0);
        // } else {
        //     bitfile_position(&ri->outd_bf, ri->offsets[i/ri->offset_step]);
        //     skip_outdegrees(ri->g, &ri->outd_bf, i%ri->offset_step);
        //     *d = read_outdegree(ri->g, &ri->outd_bf);
        //     return (0);
        // }
        return (bvgraph_call_unsupported);
    }
}

/** Access the successors of a vertex.
 *
 * This operation is not thread-safe and heavily modifies the random access
 * iterator.  (Don't worry though, it doesn't touch the underlying graph, so 
 * please do use one random iterator for each thread in your code.)
 * 
 * @param[in] ri a random access iterator for the graph
 * @param[in] x the index of the node (i in [0,g->n-1])
 * @param[out] start a pointer to internal memory for an array of length
 *                   len for the successors.  DO NOT MODIFY THIS ARRAY.
 * @param[out] len the node degree.
 * @return 0 on success
 */
int bvgraph_random_successors(bvgraph_random_iterator *ri, 
                             int64_t x, int64_t** start, uint64_t *length)
{
    if (x<0 || x >= ri->g->n) {
        return (bvgraph_vertex_out_of_range);
    }
    else if (ri->offset_step <= 0) {
        return (bvgraph_requires_offsets);
    } else {
        int64_t ref, ref_index;
        int64_t i, extra_count, block_count = 0;
        bvgraph_int_vector *block = &ri->block, *left = &ri->left, 
        *len = &ri->len;

        bvgraph *g = ri->g;
        bitfile *bf = &ri->bf;
        
        uint64_t d;     //degree
        ri->curr = x;
        int rval = position_bvgraph(ri, x, &d);
        if (rval) {
            return (rval);
        }

        *length = d;

        if (d == 0) { 
            *start = NULL;
            return (0);
        }

        // read a reference only if the window is bigger than 0
        if (g->window_size > 0) {
            ref = read_reference(g, bf);
        } else {
            ref = -1;
        }

        int64_t* temp = NULL;
        int64_t* ref_links = NULL;
        uint64_t outd_ref = 0LL;
        
        // get successors of referred node
        if (ref > 0) {
            // get the reference links first
            bvgraph_random_successors(ri, x-ref, &temp, &outd_ref);

            // copy the reference successors cause ri->successors.a might be cleared
            ref_links = malloc(sizeof(int64_t)*outd_ref);
            memcpy(ref_links, temp, sizeof(int64_t)*outd_ref);
            
            // re-assigned the original node and position to ri since ri has been changed
            ri->curr = x;
            position_bvgraph(ri, x, &d);
            if (g->window_size > 0) { ref = read_reference(g, bf); } else { ref = -1; }
        }

        int64_t interval_count;
        int64_t buf1_index, buf2_index;

        const int cyclic_buffer_size = g->window_size + 1;
        // in our case, window isn't set at all, so we need to use the position
        // method to set the file pointer, this could modify ri itself.
        // this step requires offsets
        //

        ri->outd_cache[x%ri->cyclic_buffer_size] = d;
        ri->curr_outd = d;

        if (d > (unsigned)ri->max_outd) { ri->max_outd = d; }

        int_vector_ensure_size(&ri->successors, d);
        int_vector_ensure_size(&ri->buf1, d);
        int_vector_ensure_size(&ri->buf2, d);

        ref_index = (x-ref + cyclic_buffer_size)%(cyclic_buffer_size);

        if (ref > 0) {
            // total number of successors copied and total number specified
            int64_t copied, total; 
            if ( (block_count = read_block_count(g, bf)) != 0 ) {
                // TODO: test success
                int_vector_ensure_size(&ri->block, block_count);
            }

            #ifdef MAX_DEBUG
                fprintf(stderr, "block_count = %"PRId64"\n", block_count);
            #endif 

            copied = 0; 
            total = 0;

            for (i = 0; i < block_count; i++) {
                block->a[i] = read_block(g, bf) + (i == 0 ? 0 : 1);
                // TODO: test success
                total += block->a[i];
                if (i % 2 == 0) {
                    copied += block->a[i];
                }
            }

            if (block_count%2 == 0) {
                // TODO: add check for window
                // TODO: test success
                copied += (outd_ref - total);
                ri->outd_cache[ref_index] = outd_ref;
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
                int64_t prev = 0;

                // TODO: test success
                int_vector_ensure_size(left, interval_count);
                int_vector_ensure_size(len, interval_count);
                
                // now read the intervals
                left->a[0] = prev = nat2int(bitfile_read_gamma(bf)) + x;
                len->a[0] = bitfile_read_gamma(bf) + g->min_interval_length;

                prev += len->a[0];
                extra_count -= len->a[0];
                
                for (i=1; i < interval_count; i++) {
                    left->a[i] = prev = bitfile_read_gamma(bf) + prev + 1;
                    len->a[i] = bitfile_read_gamma(bf) + g->min_interval_length;
                    prev += len->a[i];
                    extra_count -= len->a[i];
                }
            }
        }

        buf1_index = 0;
        buf2_index = 0;

        #ifdef MAX_DEBUG
        fprintf("extra_count = %"PRId64"\ninterval_count = %"PRId64"\nref = %"PRId64"\n", extra_count, interval_count, ref);
        #endif 
        // read the residuals into a buffer
        {
            int64_t prev = -1;
            int64_t residual_count = extra_count;
            while (residual_count > 0) {
                residual_count--;
                if (prev == -1) { ri->buf1.a[buf1_index++] = prev = x + nat2int(read_residual(g, bf)); }
                else { ri->buf1.a[buf1_index++] = prev = read_residual(g, bf) + prev + 1; }
            }
        }

        if (interval_count == 0) {
            // don't do anything
        }
        else
        {
            // copy the extra interval data
            for (i = 0; i < interval_count; i++)
            {
                int64_t j, cur_left = ri->left.a[i];
                for (j = 0; j < ri->len.a[i]; j++) {
                    ri->buf2.a[buf2_index++] = cur_left + j;
                }
            }
            
            if (extra_count > 0)
            {
                // merge buf1, buf2 into arcs
                merge_int_arrays(ri->buf1.a, buf1_index, ri->buf2.a, buf2_index, 
                    ri->successors.a, ri->successors.elements);

                // now copy arcs back to buffer1, and free buffer2
                buf1_index = buf1_index + buf2_index;
                buf2_index = 0;           
                memcpy(ri->buf1.a, ri->successors.a, buf1_index*sizeof(int64_t));
            }
            else
            {
                memcpy(ri->buf1.a, ri->buf2.a, buf2_index*sizeof(int64_t));
                buf1_index = buf2_index;
                buf2_index = 0;
            }
        }

        if (ref <= 0)
        {
            // don't do anything except copy
            // the data to arcs
            if (interval_count == 0 || extra_count == 0) {
                memcpy(ri->successors.a, ri->buf1.a, sizeof(int64_t)*buf1_index);
            }
        }

        else
        {          
            // TODO clean this code up          
            // copy the information from the masked iterator

            int64_t mask_index = 0;
            // this variable is intended to shadow the vector len
            int64_t len = 0;

            for (i=0; i < (signed)outd_ref; )
            {
                if (len <= 0)
                {
                    if (block_count == mask_index) 
                    {
                        if (block_count % 2 == 0) {
                            len = ri->outd_cache[ref_index] - i;
                        }
                        else {
                            break;
                        }
                    }
                    else {
                        if (mask_index % 2 == 0) { len = ri->block.a[mask_index++]; }
                        else { i += ri->block.a[mask_index++]; continue; }
                    }

                    // in the case that length is 0, we continue.
                    if (len == 0) { continue; }
                }

                ri->buf2.a[buf2_index++] = ref_links[i];
                len--;
                i++;
            }

            merge_int_arrays(ri->buf1.a, buf1_index, ri->buf2.a, buf2_index, 
                    ri->successors.a, ri->successors.elements);

            buf1_index = buf1_index + buf2_index;
            buf2_index = 0;
            
            // free the successors of referred node
            free(ref_links);
        }
    }


    if (start){
        *start = ri->successors.a;
    }
    
    return (0);

}

/** Release memory associated with the random iterator.
 *
 * @param[in] ri the random iterator
 * @return 0 on success
 */
int bvgraph_random_free(bvgraph_random_iterator* ri)
{
    int i;
    //if (ri->g && ri->curr == ri->g->n) {
    //    ri->g->max_outd = ri->max_out;
    //}

    bitfile_close(&ri->bf);
    bitfile_close(&ri->outd_bf);
    if (ri->bf.f) { fclose(ri->bf.f); }
    free(ri->outd_cache);
    int_vector_free(&ri->successors);
    for (i=0; i < ri->cyclic_buffer_size; i++) {
        int_vector_free(&ri->window[i]);
    }

    free(ri->window);
    int_vector_free(&ri->block);
    int_vector_free(&ri->left);
    int_vector_free(&ri->len);
    int_vector_free(&ri->buf1);
    int_vector_free(&ri->buf2);
    ri->g = NULL;
    return (0);
}

