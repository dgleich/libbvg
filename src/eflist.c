/**
 * @file eflist.c
 * Implement all the routines to work with eflist in the bvg library.
 * @author Yongyang Yu
 * @date 11 Aug 2014
 * @brief implementation of the routines in eflist.h
 */

#include "eflist.h"
#include <math.h>
#include <inttypes.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

/**
 * Define some error codes;
 */

const int eflist_out_of_bound = -1; ///< ef-list out of bound
const int eflist_spill_too_small = -2; ///< memory allocating for spill structure too small
const int eflist_batch_nondecreasing = -3; ///< the array is not nondecreaing in batch mode
 
/**
 * Define constants for bit operations.
 */

const uint64_t m1  = 0x5555555555555555; // binary: 0101...
const uint64_t m2  = 0x3333333333333333; // binary: 00110011..
const uint64_t m4  = 0x0f0f0f0f0f0f0f0f; // binary:  4 zeros,  4 ones ...
const uint64_t m8  = 0x00ff00ff00ff00ff; // binary:  8 zeros,  8 ones ...
const uint64_t m16 = 0x0000ffff0000ffff; // binary: 16 zeros, 16 ones ...
const uint64_t m32 = 0x00000000ffffffff; // binary: 32 zeros, 32 ones

const unsigned int MAX_ONES_PER_INVENTORY = 8192;  ///< 8K
const unsigned int MAX_SPAN = (1 << 16);
const unsigned int DEFAULT_SPILL_SIZE = 10 * 8192;  ///< default size for spill is 10*8K*8B=640KB

/**
 * Return the k-th bit in the array upper.
 * @param[in] upper the array
 * @param[in] k the k-th index
 * @return the k-th bit in the arrayp upper
 */
static int get_bitk(uint64_t *upper, int64_t k) {
    int64_t index = k >> 6;
    int64_t offset = k & ((1L << 6) - 1);
    int64_t tmp = upper[index] & (1L << offset);
    if (tmp == 0) {
        return (0);
    }
    return (1);
}

/**
 * Return the number of 1's in the 64-bit word.
 * @param[in] x the 64-bit word
 * @return the number of 1's in the word
 */
static int bit_count(int64_t x) {
    x = (x & m1 ) + ((x >>  1) & m1 ); //put count of each  2 bits into those  2 bits 
    x = (x & m2 ) + ((x >>  2) & m2 ); //put count of each  4 bits into those  4 bits 
    x = (x & m4 ) + ((x >>  4) & m4 ); //put count of each  8 bits into those  8 bits 
    x = (x & m8 ) + ((x >>  8) & m8 ); //put count of each 16 bits into those 16 bits 
    x = (x & m16) + ((x >> 16) & m16); //put count of each 32 bits into those 32 bits 
    x = (x & m32) + ((x >> 32) & m32); //put count of each 64 bits into those 64 bits 
    return x;
}


/**
 * This function builds a simple_select structure for the upper array of the EF coding represetation.
 * Current implementation is a simplified version of the `simple` algorithm. The simple_select keeps 
 * an inventory of ones at postion multiples of ceil(L * m / n), where L = MAX_ONES_PER_INVENTORY, m = 
 * num_ones, and n the length of the underlying bit array. If any two consecutive 1's are too farther away 
 * from each other (> 2^16), then a spill is allocated to record the position of each individual bit 
 * in the inventory.
 *
 * @param[in] ef the EF code structure
 * @param[in] num_ones number of 1's in the upper array, which is the number of vertices in the graph
 * @param[in] sel the simpleSelect structure
 * @param[in] spill_var_len if use variable length for spill, 0: no; 1: yes
 * @return 0 on success
 */

int simple_select_build(elias_fano_list *ef, int64_t num_ones, simple_select *sel, int spill_var_len)
{
    uint64_t length = sel->bitarraylen;
    sel->num_ones = num_ones;
    int window = length == 0 ? 1: (int)((num_ones * MAX_ONES_PER_INVENTORY + length - 1) / length);
    sel->log2_ones_per_inventory = (int)(floor(log2(window)));
    sel->ones_per_inventory = 1 << sel->log2_ones_per_inventory;
    sel->ones_per_inventory_mask = sel->ones_per_inventory - 1;
    sel->inventory_size = (int)((num_ones + sel->ones_per_inventory - 1) / sel->ones_per_inventory);
    sel->inventory = malloc(sizeof(int64_t) * (sel->inventory_size + 1));
    int64_t d = 0;
    uint64_t i;
    for (i = 0; i < length; i ++) {      
        if (bit_array_getbitk(&(ef->upper), i) == 1) {
            if ((d & sel->ones_per_inventory_mask) == 0) {
                sel->inventory[(int)(d >> sel->log2_ones_per_inventory)] = i;
            }
            d ++;
        }
    }
    sel->inventory[sel->inventory_size] = length;
    
    if (sel->ones_per_inventory > 1) {
        d = 0;
        int ones;
        uint64_t start = 0, span = 0;
        int spilled = 0, inventory_index = 0;
        for (i = 0; i < length; i ++) {
            if (bit_array_getbitk(&(ef->upper), i) == 1) {
                if ((d & sel->ones_per_inventory_mask) == 0) {
                    inventory_index = (int)(d >> sel->log2_ones_per_inventory);
                    start = sel->inventory[inventory_index];
                    span = sel->inventory[inventory_index + 1] - start;
                    ones = (int)fmin(sel->num_ones - d, sel->ones_per_inventory);

                    if (span >= MAX_SPAN) spilled += ones;
                }
                d ++;
            }
        }

        if (spilled > 0) {
            //printf("spilled = %d\n", spilled);
            // compare spill size with the pre-allocated size
            if (sel->spill_size < spilled && !spill_var_len) {
                return eflist_spill_too_small;
            }
            else if (sel->spill_size < spilled && spill_var_len != 0) {
                free(sel->exact_spill);
                sel->spill_size = spilled;
                sel->exact_spill = malloc(sizeof(int64_t) * spilled);
            }
        }
      /* else {
            sel->spill_size = 0;
        }*/
        int offset = 0;
        spilled = 0;
        d = 0;
        for (i = 0; i < length; i ++) {
            if (bit_array_getbitk(&(ef->upper), i) == 1) {
                if ((d & sel->ones_per_inventory_mask) == 0) {
                    inventory_index = (int)(d >> sel->log2_ones_per_inventory);
                    start = sel->inventory[inventory_index];
                    span = sel->inventory[inventory_index + 1] - start;
                    offset = 0;
                }
                if (span < MAX_SPAN) {
                    continue;
                }
                else {
                    if ((d & sel->ones_per_inventory_mask) == 0) {
                        sel->inventory[inventory_index] = -spilled;
                    }
                    sel->exact_spill[spilled ++] = i;
                }
                d ++;
            }
        }
    }
    return (0);
}

/**
 * This function utilizes the simpleSelect structure to compute the position of rank-th 1 in 
 * the upper array.
 *
 * @param[in] rank the rank-th 1 in the upper array
 * @param[in] sel the simpleSelect structure
 * @param[in] ef the EF code structure
 * @return the position of rank-th 1 in the upper array
 */

int64_t select_rank(uint64_t rank, simple_select *sel, elias_fano_list *ef)
{
    if (rank >= sel->num_ones) {
        return eflist_out_of_bound;
    }
    int inventory_index = (int)(rank >> sel->log2_ones_per_inventory);
    int64_t inventory_rank = sel->inventory[inventory_index];
    int subrank = (int)(rank & sel->ones_per_inventory_mask); // offset to inventoryRank
    if (subrank == 0) {
        return inventory_rank & ~(1L << 63);
    }
    
    if (inventory_rank < 0) {
        return sel->exact_spill[(int)(-inventory_rank + subrank)];
    }
    int64_t j;

    // sequential search for 1's, long word by long word
    int64_t upper_index = inventory_rank >> 6;
    int64_t offset = inventory_rank & ((1L << 6) - 1);
    int k;
    for (k = offset+1; k < 64; k ++) {
        if ((((ef->upper).A[upper_index] & (1L << k)) != 0) && (subrank != 0)) {
            subrank --;
        }
        if (subrank==0) {
            return (upper_index << 6) + k;
        }
    }
    upper_index ++;
    int ones = bit_count((ef->upper).A[upper_index]);
    while (ones < subrank) {
        subrank -= ones;
        upper_index ++;
        ones = bit_count((ef->upper).A[upper_index]);
    }
    
    for (j = 0; j < 64; j ++) {
        if ((((ef->upper).A[upper_index] & (1L << j)) != 0) && (subrank != 0)) {
            subrank --;
        }
        if (subrank == 0) {
            return (upper_index << 6) + j;
        }
    }
    return 0;
}

/**
 * get the index-th offset from the offsets array based on EF code
 * 
 * @param[in] ef the EF list structure
 * @param[in] index the index of the list
 * @return the value at the required index
 */
int64_t eflist_lookup(elias_fano_list *ef, int64_t index)
{
    int64_t lowx, highx;
    lowx = bit_array_get(&(ef->lower), index);
    highx = select_rank(index, &(ef->sel), ef);
    return ((highx - index) << ef->s) | lowx;
}

/**
 * Free all the allocated memory.
 *
 * @param[in] ef the EF code structure
 * @return 0 on success
 */

int eflist_free(elias_fano_list *ef)
{
    free((ef->sel).inventory);
    if ((ef->sel).spill_size > 0) {
        free((ef->sel).exact_spill);
    }
    bit_array_free(&(ef->lower));
    bit_array_free(&(ef->upper));
    return (0);
}

/**
 * This function sets a single bit in the bit_array.
 *
 * @param[in] ptr a pointer to the bit_array
 * @param[in] k the k-th bit to set
 * @return 0 on success
 */
int bit_array_setbit(bit_array *ptr, int64_t k)
{
    assert(ptr->s < 0);
    int64_t index = k >> 6;
    int64_t offset = k & ((1L << 6) - 1);
    ptr->A[index] |= 1L << offset;
    return 0;
}

/**
 * This function put a number of s-bits in the bit_array.
 * 
 * @param[in] ptr a pointer to the bit_array
 * @param[in] num the number to put
 * @param[in] k the k-th number
 * @return 0 on success
 */
int bit_array_insert(bit_array *ptr, uint64_t num, int64_t k)
{
    if (ptr->s == 0) {
        return 0;
    }
    assert(ptr->s > 0);
	uint64_t index = k * ptr->s / 64;
	int offset = (k * ptr->s) % 64;
	if (offset + ptr->s <= 64) { 
		uint64_t tmp = num;
		tmp = tmp << (64 - offset - ptr->s);
		ptr->A[index] |= tmp;
	}
	else {
		// need to span two words
		int diff = offset + ptr->s - 64;
		uint64_t tmp = num;
		ptr->A[index] |= tmp >> diff;
		ptr->A[index+1] |= tmp << (64-diff);
	}
	return 0;
}

/**
 * This function get the k-th s-bits number from array B.
 * 
 * @param[in] ptr a pointer to the bit_array
 * @param[in] k the k-th number to fetch
 * @return the k-th number of s-bits
 */
uint64_t bit_array_get(bit_array *ptr, int64_t k)
{
    if (ptr->s == 0) {
        return 0;
    }
	uint64_t index = k * ptr->s / 64;
	int offset = (k * ptr->s) % 64;
	if (offset + ptr->s <= 64) {
		uint64_t mask = (1L << (64 - offset)) - 1;
		mask -= (1L << (64 - offset - ptr->s)) - 1;
		uint64_t rval = ptr->A[index] & mask;
		return (rval >> (64 - offset - ptr->s));
	}
	else {
		int diff = offset + ptr->s -64;
		uint64_t mask = (1L << (ptr->s - diff)) - 1;
		uint64_t rval = ptr->A[index] & mask;
		uint64_t next_mask = 0xFFFFFFFFFFFFFFFF;
		next_mask -= (1L << (64-diff)) - 1;
		uint64_t next = ptr->A[index + 1] & next_mask;
		next >>= (64 - diff);
		rval = (rval << diff) | next;
		return rval; 
	}
}


/**
 * This function creates a new bitarray based on the given arguments.
 *
 * @param[in] ptr a pointer to a bit_array object
 * @param[in] s s-bit for each element, -1 indicates set individual bit only
 * @param[in] size the number of elements in the bit_array
 * @return 0 on success
 */
int bit_array_create(bit_array *ptr, int s, int64_t size)
{
    ptr->s = s;
    if (s == 0) {
        ptr->size = 0;
        ptr->A = NULL;
        return 0;
    }
    if (s > 0) {
        ptr->size = (s * size + 63) / 64;
    }
    else {
        ptr->size = size;
    }
    ptr->A = malloc(sizeof(uint64_t) * ptr->size);
    memset(ptr->A, 0, sizeof(uint64_t) * ptr->size);
    return 0;
}

/**
 * This function retrieves the k-th bit in the bit_array (intended to work with upper bits array only).
 * 
 * @param[in] ptr a pointer to the bit_array
 * @param[in] k k-th bit to retrieve
 * @return 0 or 1
 */
int bit_array_getbitk(bit_array *ptr, int64_t k)
{
    assert(ptr->s < 0);
    return get_bitk(ptr->A, k);
}

/**
 * This functions frees the memory allocated for the bit_array.
 *
 * @param[in] ptr a pointer to the bit_array object
 * @return 0 on success 
 */

int bit_array_free(bit_array *ptr)
{
    free(ptr->A);
    return 0;
}

/**
 * This function initializes an elias_fano_list with a desired number of elements.
 * And a default number of exact_spill buffer is allcated.
 * 
 * @param[in] ef the Elias-Fano list
 * @param[in] num_elements the total number of elements to be stored in the eflist
 * @return 0 on success
 */

int eflist_initial(elias_fano_list *ef, uint64_t num_elements)
{
    ef->size = num_elements;
    (ef->lower).remaining = num_elements;
    (ef->upper).remaining = num_elements;
    (ef->sel).spill_size = DEFAULT_SPILL_SIZE;
    (ef->sel).exact_spill = malloc(sizeof(int64_t) * DEFAULT_SPILL_SIZE);
    return 0;
}

/**
 * This function adds an element into the eflist.
 *
 * @param[in] ef the Elias-Fano list
 * @param[in] elem the element to insert
 * @param[in] 0 on success; -1 on eflist_out_of_bound
 */
int eflist_add(elias_fano_list *ef, int64_t elem)
{
    if ((ef->lower).remaining <= 0) {
        return eflist_out_of_bound;  // an error returned if too many elements
    }
    int64_t index = ef->size - (ef->lower).remaining;
    int64_t mask = (1L << ef->s) - 1;
    int64_t val = elem & mask;
    bit_array_insert(&(ef->lower), val, index);
    int64_t k = (elem >> ef->s) + index;
    bit_array_setbit(&(ef->upper), k);
    (ef->lower).remaining --;
    (ef->upper).remaining --;
    return 0;
}

/**
 * This function adds elements into an eflist in a batch mode.
 *  
 * @param[in] ef the Elias-Fano list
 * @param[in] arr the array of elements
 * @param[in] length length of the array arr
 * @return 0 on success; -1 on eflist_out_of_bound; -3 on arr is not nondecreasing
 */
int eflist_addbatch(elias_fano_list *ef, int64_t *arr, int64_t length)
{
    // test if arr is nondecreasing
    int64_t i;
    for (i = 0; i < length-1; i ++) {
        if (arr[i] <= arr[i+1]) {
            continue;
        }
        else {
            return eflist_batch_nondecreasing;
        }
    }
    // call eflist_add() to add each element to eflist
    int rval;
    for (i = 0; i < length; i ++) {
        rval = eflist_add(ef, arr[i]);
        if (!rval) {
            return rval;
        }
    }
    return 0;
}

