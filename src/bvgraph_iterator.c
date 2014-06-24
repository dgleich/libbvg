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
 * 2007-06-10: Changed iterator to allocate block,len,left,buf1,buf2 only once
 *              per iterator instead of once per next call.  This improved the
 *              iteration speed by almost 3x for cnr-2000.  Added 
 *              corresponding allocation and deallocation to 
 *              bvgraph_nonzero_iterator and bvgraph_iterator_free.
 * 2007-07-02: Fixed uninitialized error for gcc
 * 2008-03-10: Refactored read_* routines into bvgraph_io.c
 * 2008-03-11: Correctly close the bitfile file pointers
 * 2008-05-08: Set graph max_outd when closing the a valid iterator now 
 */
 
/** Todo
 * Add error checking and error returns
 * Check for performance (remove memcopies?)
 */

#include "bvgraph_internal.h"
#include "bvgraph_inline_io.h"

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
    long long outd_alloc = 10;
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
    } else if (g->offset_step == 0 || g->offset_step == 1) {
        rval = bitfile_map(g->memory, g->memory_size, &i->bf);
    } else {
        return bvgraph_call_unsupported;
    }

    // beyond this point, the bitfile was successfully allocated, so we must 
    // deallocate it if we exit.

    i->outd_cache = malloc(sizeof(long long)*i->cyclic_buffer_size);
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
    if (i->bf.f) { fclose(i->bf.f); } 
  
    return rval;
}

/**
 * to be modified.
 * Create a random access iterator for the bvgraph.  The random access iterator is 
 * a new object, which is different from non-zero iterator and used to access the 
 * outdegree or successors for any given node.  There can be many random access
 * iterators and each random access iterator is independent.
 *
 * The random access iterator requires persistent memory to work and is useful
 * for a single access over the file.  Each new access requires a new random access
 * iterator at present.
 */
int bvgraph_random_access_iterator(bvgraph* g, bvgraph_random_iterator *i)
{
    int rval = 0;
    int outd_alloc = 10;
    int windcount = 0;

    // use the default max degree because we won't persist
    // the data in most cases.
    // TODO check this for performance.
    i->max_outd = 0; 

    i->g = g;
    i->curr = -1;
    i->curr_outd = -1;
    i->cyclic_buffer_size = i->g->window_size+1;

    // for successors cache
    i->successors_cache = NULL;

    if (g->offset_step < 1) {
        return bvgraph_call_unsupported;
    }

    rval = bitfile_map(g->memory, g->memory_size, &i->bf);
    rval |= bitfile_map(g->memory, g->memory_size, &i->outd_bf);

    // TODO deallocate these on failure

    i->offset_step = 1;

    // beyond this point, the bitfile was successfully allocated, so we must 
    // deallocate it if we exit.
    i->outd_cache = malloc(sizeof(long long)*i->cyclic_buffer_size);
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
                rval |= int_vector_create(&i->left, outd_alloc);
                rval |= int_vector_create(&i->len, outd_alloc);
                rval |= int_vector_create(&i->buf1, outd_alloc);
                rval |= int_vector_create(&i->buf2, outd_alloc);
                
                if (rval == 0) {
                    // we successfully allocated everything
                    return (0);
                }

                // TODO figure out how to release i->block, i->left, i->len
                // i->buf1, i->buf2
                //

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
    if (i->bf.f) { fclose(i->bf.f); } 

    return rval;
}

/**
 * This routine provides access to the successors array stored inside
 * the iterator, so the successor array will be invalidated after 
 * a call to iterator_next.
 * ===
 * 08/28/11
 * start: starting point of links (array)
 * len: outdegree
 */
int bvgraph_iterator_outedges(bvgraph_iterator* i, long long** start, unsigned long long* len)
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
int merge_int_arrays(const long long* a1, size_t a1len, const long long* a2, 
                     size_t a2len, long long *out, size_t outlen)
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
        memcpy(&out[oi], &a1[a1i], (a1len - a1i)*sizeof(long long));
    } else {
        memcpy(&out[oi], &a2[a2i], (a2len - a2i)*sizeof(long long));
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
    const long long x = ++(iter->curr);
    long long ref = 0, ref_index = 0;
    long long i = 0, extra_count = 0, block_count = 0;

    // TODO: make these static arrays for the iterator
    // bvgraph_int_vector block, left, len, buf1, buf2;

    bvgraph *g = iter->g;
    bitfile *bf = &iter->bf;

    long long d, copied, total, interval_count;
    long long buf1_index, buf2_index;

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
    fprintf(stderr, "** begin successors\ncurr = %lu\n", iter->curr);
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
    fprintf(stderr, "block_count = %lu\n", block_count);
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
            long long prev = 0;
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
    fprintf("extra_count = %lu\ninterval_count = %lu\nref = %lu\n", extra_count, interval_count, ref);
#endif 
    // read the residuals into a buffer
    {
        long long prev = -1;
        long long residual_count = extra_count;
        while (residual_count > 0) {
            residual_count--;
            if (prev == -1) { iter->buf1.a[buf1_index++] = prev = x + nat2int(read_residual(g, bf)); }
            else { iter->buf1.a[buf1_index++] = prev = read_residual(g, bf) + prev + 1; }
        }
    }
                
    if (interval_count == 0)
    {
        // don't do anything
    }
    else
    {
        // copy the extra interval data
        for (i = 0; i < interval_count; i++)
        {
            long long j, cur_left = iter->left.a[i];
            for (j = 0; j < iter->len.a[i]; j++) {
                iter->buf2.a[buf2_index++] = cur_left + j;
            }
        }

        if (extra_count > 0)
        {
			
            // merge buf1, buf2 into arcs
            merge_int_arrays(iter->buf1.a, buf1_index, iter->buf2.a, buf2_index, 
                iter->successors.a, iter->successors.elements);
            
            // now copy arcs back to buffer1, and free buffer2
            buf1_index = buf1_index + buf2_index;
            buf2_index = 0;           
            memcpy(iter->buf1.a, iter->successors.a, buf1_index*sizeof(long long));
        }
        else
        {
            memcpy(iter->buf1.a, iter->buf2.a, buf2_index*sizeof(long long));
            buf1_index = buf2_index;
            buf2_index = 0;
        }
    }

    if (ref <= 0)
    {
		
        // don't do anything except copy
        // the data to arcs
        if (interval_count == 0 || extra_count == 0) {
            memcpy(iter->successors.a, iter->buf1.a, sizeof(long long)*buf1_index);
        }
    }
    else
    {          
        // TODO clean this code up          
        // copy the information from the masked iterator
        
        long long mask_index = 0;
        // this variable is intended to shadow the vector len
        long long len = 0;

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
        
        merge_int_arrays(iter->buf1.a, buf1_index, iter->buf2.a, buf2_index, 
            iter->successors.a, iter->successors.elements);

        buf1_index = buf1_index + buf2_index;
        buf2_index = 0;
    }

    // update the window
	
    {
        long long curr_index = iter->curr % iter->cyclic_buffer_size;
        int_vector_ensure_size(&iter->window[curr_index], d);
        
        // unwrap the buffered output
        memcpy(iter->window[curr_index].a, iter->successors.a, sizeof(long long)*d);
    }
	
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
    if (iter->g && iter->curr == iter->g->n) { 
        iter->g->max_outd = iter->max_outd;
    }
    bitfile_close(&iter->bf);
    if (iter->bf.f) { fclose(iter->bf.f); } 
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

/** Copy a bvgraph iterator structure along with all of the arrays.
 * 
 * This operation makes an exact copy of the iterator i and 
 * COULD be used to restart iteration from EXACTLY the state of i.
 * 
 * At the moment, this only works for bvgraphs loaded into memory,
 * as supporting bitfiles on disk would mean we need to be able to 
 * seek to positions in bitfiles, which isn't quite supported yet.
 * 
 * The iteration *j should be empty, as we'll allocate all arrays
 * for it.  (It should have either been freshly allocated, or had
 * bvgraph_iterator_free called on it.)
 * 
 * @param i the new iterator
 * @param j the old iterator
 */
int bvgraph_iterator_copy(bvgraph_iterator *i, bvgraph_iterator *j)
{
    int rval = 0;
    int windcount = 0;
    
    if (j->g->offset_step != 0) {
        return bvgraph_call_unsupported;
    }
    
    i->max_outd = j->max_outd;
    i->g = j->g;
    i->curr = j->curr;
    i->curr_outd = j->curr_outd;
    i->cyclic_buffer_size = j->cyclic_buffer_size;

    if (j->g->offset_step == 0) {
        memcpy(&i->bf, &j->bf, sizeof(bitfile));
    } else {
        return bvgraph_call_unsupported;
    }

    // beyond this point, the bitfile was successfully allocated, so we must 
    // deallocate it if we exit.

    i->outd_cache = malloc(sizeof(long long)*i->cyclic_buffer_size);
    if (i->outd_cache) {
        memcpy(i->outd_cache, j->outd_cache, sizeof(long long)*i->cyclic_buffer_size);
        i->window = malloc(sizeof(bvgraph_int_vector)*i->cyclic_buffer_size);
        if (i->window) {
            rval = int_vector_create_copy(&i->successors, &j->successors);
            if (rval == 0) {
                for (windcount = 0; windcount < i->cyclic_buffer_size; windcount++) {
                    rval = int_vector_create_copy(&i->window[windcount], &j->window[windcount]);
                    if (rval != 0) { break; }
                }

                // 
                // this statement here indicates we should return
                // with success and a correctly allocated 
                // interator
                //
                
                rval = int_vector_create_copy(&i->block, &j->block);
                rval |= int_vector_create_copy(&i->left, &j->left);
                rval |= int_vector_create_copy(&i->len, &j->len);
                rval |= int_vector_create_copy(&i->buf1, &j->buf1);
                rval |= int_vector_create_copy(&i->buf2, &j->buf2);
                
                if (rval == 0) {
                    return (0);                
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

/** Compute the average balanace for a set of parallel iterators.
 *
 * This function will clean up all intermediate computations
 * if it fails.
 */
static int compute_avgbalance(bvgraph *g, int niters, int wnode, int wedge, 
        long long *avgbalance)
{
    int rval; 
    unsigned long long d;
    long long balance = 0;
    bvgraph_iterator iter;

    if (!avgbalance) { return bvgraph_call_unsupported; /* TODO: better return */ }

    rval = bvgraph_nonzero_iterator(g, &iter);
    if (!rval) {
        for (; bvgraph_iterator_valid(&iter); bvgraph_iterator_next(&iter)) {
            bvgraph_iterator_outedges(&iter, NULL, &d);
            balance += wnode + wedge*d;
        }
        bvgraph_iterator_free(&iter);

        *avgbalance = (balance + niters-1)/niters; /* round by the ceil function */

        if (BVGRAPH_VERBOSE) {
            fprintf(stdout,"average balance for %3i iters is %9lli\n",
                niters, *avgbalance);
        }

        return (0);
    }

    return (rval);
}

/** Distribute iterators trying to maintain a balanace
 *
 * The algorithm for distributing iterators it to keep
 * adding nodes from the graph until the total balance
 * on the current iterator exceeds the average balanace passed
 * in avgbalance.
 * 
 * This function will clean up all intermediate computations
 * if it fails.
 */
static int distribute_iters(bvgraph *g, bvgraph_parallel_iterators *pits,
        int wnode, int wedge, long long avgbalance)
{
    int rval, nsteps, iter;
    unsigned long long d;
    long long balance = 0; 
    bvgraph_iterator git;

    /* clear iterators so we can use iter->g to test for success/failure */
    for (iter=0; iter<pits->niters; iter++) {
        memset(&pits->iters[iter], 0, sizeof(bvgraph_iterator));
    }

    rval = bvgraph_nonzero_iterator(g, &git);
    if (!rval) {
        iter = 0; nsteps = 0;
        rval = bvgraph_iterator_copy(&pits->iters[iter], &git);
        if (!rval) {
            while (bvgraph_iterator_valid(&git)) {
                if (balance >= avgbalance) {
                    pits->nsteps[iter] = nsteps;
                    if (BVGRAPH_VERBOSE) {
                        fprintf(stdout,
                            "balance on iterator %3i is %9lli with %9d steps\n",
                            iter, balance, nsteps);
                    }
                    iter++;
                    rval = bvgraph_iterator_copy(&pits->iters[iter], &git);
                    nsteps = 0; balance = 0;
                    if (rval) { iter--; break; }
                }
                bvgraph_iterator_outedges(&git, NULL, &d);
                balance += wnode + wedge*d;
                bvgraph_iterator_next(&git); nsteps++;
            }
            bvgraph_iterator_free(&git);

            if (!rval) {
                /* nothing went wrong inside the allocation */
                pits->nsteps[iter] = nsteps;
                if (BVGRAPH_VERBOSE) {
                    fprintf(stdout,
                        "balance on iterator %3i is %9lli with %9d steps\n",
                        iter, balance, nsteps);
                }

                iter++;
                if (iter < pits->niters) {
                    if (BVGRAPH_VERBOSE) { 
                        fprintf(stdout, 
                            "could only allocate %3i iterators and not %3i\n",
                            iter, pits->niters);
                    }
                    pits->niters = iter;
                }

                return (0);
            } 

            /* free all allocated iterators, something went wrong */
            while (iter) {
                bvgraph_iterator_free(&pits->iters[iter--]);
            }
        }
        bvgraph_iterator_free(&git);

        return (0);
    }

    return (rval);
}

/** Construct independent iterators over portions of the graph.
 * 
 * These iterators are most often used to do parallel iteration 
 * over the graph, hence the term.
 * 
 * The graph is divided into a series of iterators by trying
 * to balance wnode*nnodes + wedge*nedges on each of the iterators.
 * 
 * For OpenMP tasks, wnode=0, wedge=1 is often a good choice.
 * For MPI taks, wnode=1, wedge=1 is often a good choice.
 * 
 * @param g the bvgraph
 * @param pits an uninitialized bvgraph_parallel_iterators structure
 * to hold the set of iterators
 * @param niters the number of iterators to create
 * @param wnode a weight applied to each node to balance the work
 * @param wedge a weight applied to each edge to balance the work
 */
int bvgraph_parallel_iterators_create(bvgraph *g, 
        bvgraph_parallel_iterators *pits, int niters, int wnode, int wedge)
{
    int rval; 
    /* check compatibility */
    if (g->offset_step != 0) {
        return (bvgraph_call_unsupported);
    }
    /* allocate memory */
    memset(pits, 0, sizeof(bvgraph_parallel_iterators));
    pits->g = g;
    pits->niters = niters;
    pits->iters = malloc(sizeof(bvgraph_iterator)*pits->niters);
    if (pits->iters) {
        pits->nsteps = malloc(sizeof(int)*pits->niters);
        if (pits->nsteps) {
            long long avgbalance=0;
            rval = compute_avgbalance(g, niters, wnode, wedge, &avgbalance);
            if (!rval) {
                rval = distribute_iters(g, pits, wnode, wedge, avgbalance);
                if (!rval) {
                    return (0);
                }
            }
            free(pits->nsteps);
        } else {
            rval = bvgraph_call_out_of_memory;
        }
        free(pits->iters);
    } else {
        rval = bvgraph_call_out_of_memory;
    }
    return (rval);
}

/** Pick a parallel iterator from the set.
 *
 * Create a copy of the ith parallel iterator suitable for actually 
 * iterating over the graph.
 *
 * You should NOT use the contents of the parallel_iterators directly!
 * 
 * You are responsible for calling bvgraph_iterator_free() on the iterator.
 * 
 * @param pits the set of parallel iterators
 * @param iter the newly allocated iterator starting from the ith parallel 
 * iterator
 * @param i the index of the iterator in the parallel set
 * @param nsteps the number of steps to take with this iterator before
 * hitting the next parallel iterator.
 */
int bvgraph_parallel_iterator(bvgraph_parallel_iterators *pits, int i,
        bvgraph_iterator *iter, int *nsteps)
{
    int rval = bvgraph_call_unsupported; /* TODO: Change to invalid index */
    if (i<pits->niters) {
        rval = bvgraph_iterator_copy(iter, &pits->iters[i]);
        if (!rval && nsteps) {
            *nsteps = pits->nsteps[i];
        }
    }
    return (rval);
}

/** Release the collection of parallel iterators
 *
 * You should only use this function once you are done with all operations from
 * a set of parallel iterators.
 * 
 * @param pits the set to release
 */
int bvgraph_parallel_iterators_free(bvgraph_parallel_iterators *pits)
{
    int i;
    for (i=0; i<pits->niters; i++) {
        bvgraph_iterator_free(&pits->iters[i]);
    }
    free(pits->iters);
    free(pits->nsteps);
    return (0);
}


