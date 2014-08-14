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
    int64_t size;  ///< number of 64-bit long words
    uint64_t remaining; ///< remaining number of elements that can be held
};    

/**
 * Define data structure for the Elias-Fano list index.
 */

struct simple_select_tag {
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

/**
 * Define data structure for the Elias-Fano list.
 */

struct elias_fano_list_tag {
    int s;
    uint64_t size;  ///< number of elements 
    struct bit_array_tag lower;
    struct bit_array_tag upper;
    struct simple_select_tag sel;
};
    
typedef struct simple_select_tag simple_select;
typedef struct elias_fano_list_tag elias_fano_list;
typedef struct bit_array_tag bit_array;

// define some consts for simple select structure
extern const uint64_t m1;
extern const uint64_t m2;
extern const uint64_t m4;
extern const uint64_t m8;
extern const uint64_t m16;
extern const uint64_t m32;
extern const unsigned int MAX_ONES_PER_INVENTORY;
extern const unsigned int MAX_SPAN; 
extern const unsigned int DEFAULT_SPILL_SIZE;

// define some error codes for the eflist
extern const int eflist_out_of_bound;
extern const int eflist_spill_too_small;  
extern const int eflist_batch_nondecreasing; 

// function prototypes 
int simple_select_build(elias_fano_list *ef, int64_t num_ones, simple_select *sel, int spill_var_len);
int64_t select_rank(uint64_t rank, simple_select *sel, elias_fano_list *ef);
int64_t eflist_lookup(elias_fano_list *ef, int64_t index);
int eflist_free(elias_fano_list *ef);
int bit_array_create(bit_array *ptr, int s, int64_t size);
int bit_array_free(bit_array *ptr);
int bit_array_insert(bit_array *ptr, uint64_t num, int64_t k);
uint64_t bit_array_get(bit_array *ptr, int64_t k);
int bit_array_setbit(bit_array *ptr, int64_t k);
int bit_array_getbitk(bit_array *ptr, int64_t k);
int eflist_initial(elias_fano_list *ef, uint64_t num_elements);
int eflist_add(elias_fano_list *ef, int64_t elem);
int eflist_addbatch(elias_fano_list *ef, int64_t *arr, int64_t length);
    
#ifdef __cplusplus    
}
#endif

#endif //LIBBVG_EFLIST_H