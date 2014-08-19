#ifndef LIBBVG_EFLIST_H
#define LIBBVG_EFLIST_H

/**
 * @file eflist.h
 * Include a few functions with Elias-Fano nondecreasing list encodes.
 * @author Yongyang Yu
 * @date 11 Aug 2014
 * @brief Include a few fucntions which provides implementation of Elias-Fano encoding/decoding 
 * methods for nondecreasing lists.
 */

#include "bitfile.h"

#ifdef __cplusplus
extern "C" {
#endif
    
/**
 * Define data structure for equi-length bit array.
 */
struct bit_array_tag {
    int s;         ///< each element takes s-bits
    uint64_t *A;
    uint64_t size; ///< number of elements in the bit array
};    

/**
 * Define data structure for the Elias-Fano list.
 */

struct elias_fano_list_tag {
    int s;
    uint64_t curr;  ///< index of the current element
    uint64_t size;  ///< number of elements 
    struct bit_array_tag lower;
    struct bit_array_tag upper;
    uint64_t largest;
    // below defines the index simple_select index structure for eflist
    int log2_ones_per_inventory;
    int ones_per_inventory;
    int ones_per_inventory_mask;
    int inventory_size;
    uint64_t spill_size;
    uint64_t spill_curr;
    int64_t *inventory;
    int64_t *exact_spill;   
};

typedef struct elias_fano_list_tag elias_fano_list;
typedef struct bit_array_tag bit_array; 

// function prototypes 
int bit_array_create(bit_array *ptr, int s, int64_t size);
int bit_array_free(bit_array *ptr);
int bit_array_put(bit_array *ptr, uint64_t num, int64_t k);
uint64_t bit_array_get(bit_array *ptr, int64_t k);
int eflist_create(elias_fano_list *ef, uint64_t num_elements, uint64_t largest);
int eflist_add(elias_fano_list *ef, int64_t elem);
int eflist_addbatch(elias_fano_list *ef, int64_t *arr, int64_t length);
int64_t eflist_get(elias_fano_list *ef, int64_t index);
int eflist_free(elias_fano_list *ef);
size_t eflist_size(elias_fano_list *ef);
int64_t bit_search(void *mem, int64_t start_bit_offset, size_t l);
    
#ifdef __cplusplus    
}
#endif

#endif //LIBBVG_EFLIST_H