/*
 * David Gleich
 * Copyright, Stanford University, 2008
 * 10 March 2008
 */

/**
 * @file bvgraph_random.c
 * Implement the set of routines to work with the bvgraph 
 * with random access, but only when offset_step = 1
 */

/** History
 *
 * 2008-03-10: Coding started
 */

#include "bvgraph_internal.h"
#include "bvgraph_inline_io.h"

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
static int skip_node(bvgraph_random_iterator *ri,
                     int x, unsigned int d)
{
    return (bvgraph_call_unsupported);
}

/** Positions the given input bit stream exactly before the successor list of the
 * given node, just after the outdegree, which is returned.  
 *
 * @param ri the random iterator
 * @param x the index of the node
 * @param[out] d the outdegree
 */
static int position_bvgraph(bvgraph_random_iterator *ri, int x, unsigned int *d)
{
    if (x < 0 || x >= ri->g->n) {
        return (bvgraph_vertex_out_of_range);
    }

    if (ri->offset_step <= 0) {
        return bvgraph_requires_offsets;
    } else if (ri->offset_step == 1) {
        int rval = bitfile_position(&ri->bf, ri->offsets[x]);
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
 * @param ri a random access iterator for the graph
 * @param i the index of the node (i in [0,g->n-1])
 * @param[out] d the node degree.
 */
int bvgraph_random_outdegree(bvgraph_random_iterator *ri, 
                             int i, unsigned int *d)
{
    if (i<0 || i >= ri->g->n) {
        return (bvgraph_vertex_out_of_range);
    }

    // TODO: always add outd_cache search

    if (ri->offset_step <= 0) {
        return (bvgraph_requires_offsets);
    } else if (ri->offset_step == 1) {
        bitfile_position(&ri->outd_bf, ri->offsets[i]);
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
 * @param ri a random access iterator for the graph
 * @param x the index of the node (i in [0,g->n-1])
 * @param[out] start a pointer to internal memory for an array of length
 *                   len for the successors.  DO NOT MODIFY THIS ARRAY.
 * @param[out] len the node degree.
 */
int bvgraph_random_successors(bvgraph_random_iterator *ri, 
                             int x, int** start, unsigned int *len)
{
    if (i<0 || i >= ri->g->n) {
        return (bvgraph_vertex_out_of_range);
    else if (ri->offset_step <= 0) {
        return (bvgraph_requires_offsets);
    } else {
        const int ref, ref_index;
        int i, extra_count, block_count = 0;
        bvgraph_int_vector *block = &ri->block, *left = &ri->block, 
            *len = &ri->len;

        bvgraph *g = iter->g;
        bitfile *bf = &iter->bf;

        int d, copied, total, interval_count;
        int buf1_index, buf2_index;

        const int d, cyclic_buffer_size = g->window_size + 1;
        // in our case, window isn't set at all, so we need to use the position
        // method to set the file pointer, this could modify ri itself.
        // this step requires offsets
        bvgraph_position(ri, x, &d);

        if (d == 0) { 
            *len = 0; 
            *start = NULL;
            return (0);
        }

        int_vector_ensure_size(&iter->successors, d);
        int_vector_ensure_size(&iter->buf1, d);
        int_vector_ensure_size(&iter->buf2, d);

        // read a reference only if the window is bigger than 0
        if (g->window_size > 0) {
            ref = read_reference(g, bf);
        } else {
            ref = -1;
        }
        ref_index = (x-ref + cyclic_buffer_size)%(cyclic_buffer_size);

        if (ref > 0) {
            // total number of successors copied and total number specified
            int copied, total; 
            if ( (block_count = read_block_count(g, bf)) != 0 ) {
                // TODO: test success
                int_vector_ensure_size(&iter->block, block_count);
            }

            #ifdef MAX_DEBUG
                fprintf(stderr, "block_count = %i\n", block_count);
            #endif 
            for (i = 0; i < block_count; i++) {
                copied = 0; 
                total = 0;

                for (i = 0; i < block_count; i++) {
                    block.a[i] = read_block(g, bf) + (i == 0 ? 0 : 1);
                    // TODO: test success
                    total += block.a[i];
                    if (i % 2 == 0) {
                        copied += block.a[i];
                    }
                }
                if (block_count%2 == 0) {
                    // TODO: add check for window
                    unsigned int outd_ref;
                    bvgraph_random_outdegree(ri, x-ref, &outd_ref);
                    // TODO: test success
                    copied += (outd_ref - total);
                }
                // TODO: error on copied > d
                extra_count = d - copied;
            }
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
                int_vector_ensure_size(left, interval_count);
                int_vector_ensure_size(len, interval_count);
                
                // now read the intervals
                left.a[0] = prev = nat2int(bitfile_read_gamma(bf)) + x;
                en.a[0] = bitfile_read_gamma(bf) + g->min_interval_length;
                
                prev += len.a[0];
                extra_count -= len.a[0];
                
                for (i=1; i < interval_count; i++) {
                    left.a[i] = prev = bitfile_read_gamma(bf) + prev + 1;
                    len.a[i] = bitfile_read_gamma(bf) + g->min_interval_length;
                    prev += len.a[i];
                    extra_count -= len.a[i];
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
                if (prev == -1) { ri->buf1.a[buf1_index++] = prev = x + nat2int(read_residual(g, bf)); }
                else { ri->buf1.a[buf1_index++] = prev = read_residual(g, bf) + prev + 1; }
            }
            // std::cout << "residuals: buf1" << std::endl;
            // std::copy(buffer1.begin(),buffer1.begin()+buf1_index,std::ostream_iterator<int>(std::cout, "\n"));
        }
        // std::cout << "buf1_index = " << buf1_index << std::endl;

        if (interval_count == 0) {
            // don't do anything
        }
        else
        {
            // copy the extra interval data
            for (i = 0; i < interval_count; i++)
            {
                int j, cur_left = ri->left.a[i];
                for (j = 0; j < ri->len.a[i]; j++) {
                    ri->buf2.a[buf2_index++] = cur_left + j;
                }
            }
            
            // std::cout << "sequences: buf2" << std::endl;
            // std::copy(buffer2.begin(),buffer2.begin()+buf2_index,std::ostream_iterator<int>(std::cout, "\n"));
            
            if (extra_count > 0)
            {
                // merge buf1, buf2 into arcs
                merge_int_arrays(ri->buf1.a, buf1_index, ri->buf2.a, buf2_index, 
                    ri->successors.a, ri->successors.elements);
                
                // now copy arcs back to buffer1, and free buffer2
                buf1_index = buf1_index + buf2_index;
                buf2_index = 0;           
                memcpy(ri->buf1.a, ri->successors.a, buf1_index*sizeof(int));
            }
            else
            {
                memcpy(ri->buf1.a, ri->buf2.a, buf2_index*sizeof(int));
                buf1_index = buf2_index;
                buf2_index = 0;
            }
        }

        /*
		try {		
			final int residualCount = extraCount; // Just to be able to use an anonymous class.

			final LazyIntIterator residualIterator = residualCount == 0 ? null : new ResidualIntIterator( this, ibs, residualCount, x );
			
			// The extra part is made by the contribution of intervals, if any, and by the residuals iterator.
			final LazyIntIterator extraIterator = intervalCount == 0 
				? residualIterator 
				: ( residualCount == 0 
					? (LazyIntIterator)new IntIntervalSequenceIterator( left, len )
					: (LazyIntIterator)new MergedIntIterator( new IntIntervalSequenceIterator( left, len ), residualIterator )
					);

			final LazyIntIterator blockIterator = ref <= 0
				? null 
				: new MaskedIntIterator(
										// ...block for masking copy and...
										block, 
										// ...the reference list (either computed recursively or stored in window)...
										window != null 
										? LazyIntIterators.wrap( window[ refIndex ], outd[ refIndex ] )
										: 
											// This is the recursive lazy part of the construction.
											successors( x - ref, isMemory ? new InputBitStream( graphMemory ) : new InputBitStream( new FastMultiByteArrayInputStream( graphStream ) ), null, null, null )
										);
			
			if ( ref <= 0 ) return extraIterator;
			else return extraIterator == null
					 ? blockIterator
					 : (LazyIntIterator)new MergedIntIterator( blockIterator, extraIterator, d );
			
		}
		catch( IOException cantHappen ) { LOGGER.fatal("Accessing node " + x, cantHappen ); throw new RuntimeException( cantHappen ); }
		*/
    
}

/** Release memory associated with the random iterator.
 *
 * @param ri the random iterator
 * @return 0 on success
 */
int bvgraph_random_free(bvgraph_random_iterator* ri)
{
}

