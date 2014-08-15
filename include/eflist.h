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
    int s;
    uint64_t *A;
    uint64_t curr; ///< curr index of next element to be inserted
};    


/**
 * Define data structure for the Elias-Fano list.
 */

struct elias_fano_list_tag {
    int s;
    uint64_t size;  ///< number of elements 
    struct bit_array_tag lower;
    struct bit_array_tag upper;
    // below defines the index simple_select index structure for eflist
    int log2_ones_per_inventory;
    int ones_per_inventory;
    int ones_per_inventory_mask;
    int inventory_size;
    uint64_t num_ones;
    uint64_t bitarraylen;
    int spill_size;
    int64_t *inventory;
    int64_t *exact_spill;   
};

typedef struct elias_fano_list_tag elias_fano_list;
typedef struct bit_array_tag bit_array;

// define some consts for simple select structure
extern const unsigned int MAX_ONES_PER_INVENTORY; 
extern const unsigned int DEFAULT_SPILL_SIZE;

// define some error codes for the eflist
extern const int eflist_spill_too_small;  

// function prototypes 
int simple_select_build(elias_fano_list *ef, int64_t num_ones, int spill_var_len);
int64_t eflist_lookup(elias_fano_list *ef, int64_t index);
int eflist_free(elias_fano_list *ef);
int bit_array_create(bit_array *ptr, int s, int64_t size);
int bit_array_free(bit_array *ptr);
int bit_array_insert(bit_array *ptr, uint64_t num, int64_t k);
uint64_t bit_array_get(bit_array *ptr, int64_t k);
int bit_array_setbit(bit_array *ptr, int64_t k);
int bit_array_getbitk(bit_array *ptr, int64_t k);
int eflist_init(elias_fano_list *ef, uint64_t num_elements, uint64_t largest);
int eflist_add(elias_fano_list *ef, int64_t elem);
int eflist_addbatch(elias_fano_list *ef, int64_t *arr, int64_t length);
    
#ifdef __cplusplus    
}
#endif

#endif //LIBBVG_EFLIST_H