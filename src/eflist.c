/**
 * @file eflist.c
 * Implement all the routines to work with eflist in the bvg library.
 * @author Yongyang Yu
 * @date 11 Aug 2014
 * @brief implementation of the routines in eflist.h
 */

#include "eflist.h"
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

/**
 * Define some error codes;
 */

const int eflist_out_of_bound = -1; ///< ef-list out of bound
const int eflist_batch_nondecreasing = -2; ///< the array is not nondecreaing in batch mode
 
/**
 * Define constants for bit operations.
 */

const uint64_t m1  = 0x5555555555555555; // binary: 0101...
const uint64_t m2  = 0x3333333333333333; // binary: 00110011..
const uint64_t m4  = 0x0f0f0f0f0f0f0f0f; // binary:  4 zeros,  4 ones ...
const uint64_t m8  = 0x00ff00ff00ff00ff; // binary:  8 zeros,  8 ones ...
const uint64_t m16 = 0x0000ffff0000ffff; // binary: 16 zeros, 16 ones ...
const uint64_t m32 = 0x00000000ffffffff; // binary: 32 zeros, 32 ones

// constants for simple select index structure
const unsigned int MAX_ONES_PER_INVENTORY = 8192;  ///< 8K
const unsigned int MAX_SPAN = (1 << 16);

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
 * @param[in] ef the EF list structure
 * @return 0 on success
 */

static int simple_select_build(elias_fano_list *ef)
{
    int64_t curr_index = ef->curr;
    uint64_t length = (ef->upper).size;
    uint64_t i;
    int64_t start;
    int inventory_index = 0;
    if (curr_index == 0) {
        ef->inventory[ef->inventory_size] = length;  // set the last element on first visit
        ef->inventory[(int)(curr_index >> ef->log2_ones_per_inventory)] = bit_search((void *)((ef->upper).A), 0, -1, 0);
    }
    else {
        if ((curr_index & ef->ones_per_inventory_mask) == 0) {
            inventory_index = (int)(curr_index >> ef->log2_ones_per_inventory);
            if (ef->inventory[inventory_index - 1] < 0) {
                printf("ERROR: ef->inventory[inventory_index - 1] < 0\n");
            }
            start = ef->inventory[inventory_index - 1];         
            ef->inventory[inventory_index] = bit_search((void *)((ef->upper).A), (inventory_index-1) * ef->ones_per_inventory, start, ef->ones_per_inventory);
       }
    }
     
// since we only have partial information about inventory, we cannot proceed as follows.    
    if (ef->ones_per_inventory > 1) {
        uint64_t span = 0;
        inventory_index = (int)(curr_index >> ef->log2_ones_per_inventory);
        if (inventory_index > 0 && ((curr_index & ef->ones_per_inventory_mask) == 0)) {
            // compute the span of inventory[inventory_index]
            start = 0;
            if (ef->inventory[inventory_index - 1] < 0) {
                printf("ERROR: ef->inventory[inventory_index - 1] < 0\n");
            } 
            start = ef->inventory[inventory_index - 1];
            span = ef->inventory[inventory_index] - start;
            if (span >= MAX_SPAN) {
                // need to spill
                if (ef->spill_size - ef->spill_curr < ef->ones_per_inventory) {
                    // double the size of current spill size
                    int64_t *tmp = ef->exact_spill;
                    ef->exact_spill = malloc(sizeof(int64_t) * ef->spill_size * 2);
                    memset(ef->exact_spill, 0, sizeof(int64_t) * ef->spill_size * 2);
                    memcpy(ef->exact_spill, tmp, ef->spill_size);
                    free(tmp);
                    ef->spill_size *= 2;
                }
                start = ef->inventory[inventory_index - 1];                
                for (i = 0; i < ef->ones_per_inventory; i ++) {
                    if ((i & ef->ones_per_inventory_mask) == 0) {
                        ef->inventory[inventory_index - 1] = -ef->spill_curr;
                    }                    
                    ef->exact_spill[ef->spill_curr ++] = bit_search((void *)(ef->upper).A, (inventory_index-1)*ef->ones_per_inventory, start, i);
                }
                
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
 * @param[in] ef the EF code structure
 * @return the position of rank-th 1 in the upper array
 */

static int64_t select_rank(uint64_t rank, elias_fano_list *ef)
{
    if (rank >= ef->size) {
        return eflist_out_of_bound;
    }
    int inventory_index = (int)(rank >> ef->log2_ones_per_inventory);
    int64_t inventory_rank = ef->inventory[inventory_index];
    int subrank = (int)(rank & ef->ones_per_inventory_mask); // offset to inventoryRank
    if (subrank == 0) {
        return inventory_rank & ~(1L << 63);
    }
    
    if (inventory_rank < 0) {
        return ef->exact_spill[(int)(-inventory_rank + subrank)];
    }
    int64_t j;

    // sequential search for 1's, long word by long word
    int64_t upper_index = inventory_rank >> 6;
    int64_t offset = inventory_rank & ((1L << 6) - 1);
    int k;
    uint64_t left1 = 0x8000000000000000;
    for (k = offset+1; k < 64; k ++) {
        if ((((ef->upper).A[upper_index] & (left1 >> k)) != 0) && (subrank != 0)) {
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
        if ((((ef->upper).A[upper_index] & (left1 >> j)) != 0) && (subrank != 0)) {
            subrank --;
        }
        if (subrank == 0) {
            return (upper_index << 6) + j;
        }
    }
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
int bit_array_put(bit_array *ptr, uint64_t num, int64_t k)
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
    assert(s >= 0);
    ptr->s = s;
    ptr->size = size;
    int64_t array_len = 0;
    if (s == 0) {
        ptr->A = NULL;
        return 0;
    } 
    if (s > 0) {
        array_len = (s * size + 63) / 64;
    }
    ptr->A = malloc(sizeof(uint64_t) * array_len);
    memset(ptr->A, 0, sizeof(uint64_t) * array_len);
    return 0;
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
    ptr->A = NULL;
    return 0;
}

/**
 * The function builds the Elias-Fano representation of a monotonously non-decreasing sequence.
 * EF coding is used to encode a monotone nondecreasing natrual number 
 * sequence x0, x1, ..., x_{n-1}. Suppose all the numbers are smaller 
 * than an upper bound u. Then EF representation can encode the sequence 
 * using at most 2 + log(u / n) bits. A typical usage: suppose we have 
 * a list of pointers, which point to records in a large file. Instead 
 * of using many bits to express the length of the file for the pointer, 
 * EF coding makes it possible to use roughly equal length of bits to 
 * represent pointers. The length equals to the logarithm of the average 
 * length of a record in the file. 
 *
 * Each element is stored separately, the lower s bits and the remaining 
 * upper bits, where s = floor(log(u/n)). The lower bits are stored contiguously 
 * while the upper bits are stored in an array U of size (n + x_{n-1} >> s) 
 * bits. Suppose k = x_{i} >> s + i, then set U[k] = 1. For example, 
 * x0=5, x1=10, x2=15, x3=20. 
 * Thus, n = 4, u = 21, s = floor(log(21/4)) = 1.  
 *
 * The length of U is 4 + 20 >> s = 14. 
 * k0 = x0 >> s + 0 = 2, U[2] = 1. Similarly, U[6] = 1, U[9] = 1, and U[13] = 1. 
 * The lower bits are stored as [1, 0, 1, 0]. To recover the original higher 
 * bits, one needs to select the i-th bit in U and subtract i. To recover 
 * x1 in the previous example, we need to select 1st 1 in U and the position 
 * is 6. By subtracting 1, we get 5 (101b) for upper bits. Combining with 
 * lower bit 0, we get x1 = 1010b = 10.
 *
 * This function initializes an elias_fano_list with a desired number of elements.
 * And a default number of exact_spill buffer is allcated.
 * 
 * @param[in] ef the Elias-Fano list
 * @param[in] num_elements the total number of elements to be stored in the eflist
 * @param[in] largest the largest element in the list
 * @return 0 on success
 */

int eflist_create(elias_fano_list *ef, uint64_t num_elements, uint64_t largest)
{
    memset(ef, 0, sizeof(elias_fano_list));
    ef->size = num_elements;
    ef->largest = largest;
    ef->inventory = NULL;
    ef->exact_spill = NULL;
    ef->curr = 0;
    // set fields for lower and upper bit arrays
    int s = num_elements == 0 ? 0 : (int)(log2( (largest + 1) / num_elements));
    ef-> s = s;
    bit_array_create(&(ef->lower), s, num_elements);
    int64_t upper_length = num_elements + (largest >> s);
    bit_array_create(&(ef->upper), 1, upper_length);
    // set fields for the index structure
    int window = upper_length == 0 ? 1 : (int)((num_elements * MAX_ONES_PER_INVENTORY + upper_length - 1) / upper_length);
    ef->log2_ones_per_inventory = (int)(floor(log2(window)));
    ef->ones_per_inventory = 1 << ef->log2_ones_per_inventory;
    ef->ones_per_inventory_mask = ef->ones_per_inventory - 1;
    ef->inventory_size = (int)((num_elements + ef->ones_per_inventory - 1) / ef->ones_per_inventory);
    //ef->num_ones = num_elements;
    ef->spill_size = ef->inventory_size;
    ef->spill_curr = 0;  // current index of spill buffer
    ef->inventory = malloc(sizeof(int64_t) * (ef->inventory_size + 1));  // allocate one more space
    // by default, set spill to be the same size as inventory 
    ef->exact_spill = malloc(sizeof(int64_t) * ef->spill_size);
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
    if (ef->curr >= ef->size) {
        return eflist_out_of_bound;  // an error returned if too many elements
    }
    int64_t index = ef->curr;
    int64_t mask = (1L << ef->s) - 1;
    int64_t val = elem & mask;
    bit_array_put(&(ef->lower), val, index);
    int64_t k = (elem >> ef->s) + index;
    bit_array_put(&(ef->upper), 1, k);
    simple_select_build(ef);
    ef->curr ++;
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
        if (rval) {
            return rval;
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
int64_t eflist_get(elias_fano_list *ef, int64_t index)
{
    if (index >= ef->curr) {
        return eflist_out_of_bound;
    }
    int64_t lowx, highx;
    lowx = bit_array_get(&(ef->lower), index);
    highx = select_rank(index, ef);
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
    free(ef->inventory);
    ef->inventory = NULL;
    if (ef->spill_size > 0) {
        free(ef->exact_spill);
        ef->exact_spill = NULL;
    }
    bit_array_free(&(ef->lower));
    bit_array_free(&(ef->upper));
    return (0);
}

/** 
 * This function computes how much memory is required for an eflist.
 * 
 * @param[in] ef a pointer to an elias_fano list
 * @return the memory required for the list in terms of bytes
 */
size_t eflist_size(elias_fano_list *ef)
{
    size_t rval = 0;
    rval += (ef->s * ef->size + 63) / 64 * 8;    // number of bytes for lower bits array
    int64_t upper_length = ef->size + (ef->largest >> ef->s);
    rval += (upper_length + 63) / 64 * 8; // number of bytes for upper bits array
    rval += ef->inventory_size * 8;   // number of bytes for inventory array
    rval += ef->spill_size * 8;     // number of bytes for spill array
    return rval;
}

/**
 * This function compute the (k+l)-th bit location from a given chunk of memory.
 *
 * @param[in] mem starting address of the memory
 * @param[in] k the k-th bit, k >= 0
 * @param[in] k_offset the offset of k-th bit when k_offset >=0
 * @param[in] l l-bits away from k-th bit, l >= 0
 * @return the (k+l)-th bit offset in the memory when k_offset >=0;
 *         the k-th bit offset in the memory if k_offset < 0
 */
int64_t bit_search(void *mem, size_t k, int64_t k_offset, size_t l)
{
    int64_t rval, i, j, index, offset;
    uint64_t *A = (uint64_t *)mem;
    size_t count = 0, goal;
    const uint64_t left1 = 0x8000000000000000;
    if (k_offset < 0) {   // look for the offset of k-th bit
        index = 0;
        rval = 0;
        count = bit_count(A[index]);
        goal = k + 1;
        while (count < goal) {
            goal -= count;
            index ++;
            count = bit_count(A[index]);
            rval += 64;
        }
        // find the word
        for (i = 0; i < 64; i ++) {
            if (goal > 0) {
                if ((A[index] & (left1 >> i)) != 0) {
                    goal --;
                }
            }
            if (goal == 0) { // k-th bit is found
                rval += i;
                return rval;
            }
        }
    }
    else {   // offset for k-th bit is given
        if (l == 0) {
            return k_offset;
        }
        index = k_offset / 64;
        offset = k_offset % 64;
        goal = l;
        rval = k_offset;
        for (j = offset + 1; j < 64; j ++) {
            if (goal > 0) {
                if ((A[index] & (left1 >> j)) != 0) {
                    goal --;
                }
            }
            if (goal == 0) {
                rval += j - offset;
                return rval;
            }
        }
        index ++;
        rval = index * 64;
        count = bit_count(A[index]);
        while (count < goal) {
            goal -= count;
            index ++;
            count = bit_count(A[index]);
            rval += 64;
        }
        for (i = 0; i < 64; i ++) {
            if (goal > 0) {
                if ((A[index] & (left1 >> i)) != 0) {
                    goal --;
                }
            }
            if (goal == 0) {
                rval += i;
                return rval;
            }
        }
    }
    return (-1);
}