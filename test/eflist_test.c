/**
 * @file eflist_test.c
 * Create an array consisting of 10000 natrual numbers in non-decreasing order.
 * Encode the array in Elias-Fano format and compare the eflist values with 
 * the original array.
 */

#include "eflist.h"
#include <stdlib.h>
#include <time.h>
#define N 10000
#define M 50

int main(int argc, char **argv)
{
    int64_t *A = NULL;
    int64_t i = 0;
    A = malloc(sizeof(int64_t) * N);
    srand(time(NULL));
    A[0] = rand() % M;
    for (i = 1; i < N; i ++) {
        A[i] = A[i-1] + rand() % M;
    }
    elias_fano_list eflist;
    eflist_create(&eflist, N, A[N-1]);
    eflist_addbatch(&eflist, A, N);
    // compare values
    for (i = 0; i < N; i ++) {
        if (A[i] != eflist_get(&eflist, i)) {
            printf("ERROR: eflist test failed!\n");
            return (-1);
        }
    }
    eflist_free(&eflist);
    free(A);
    printf("Testing eflist ... passed!\n");
    return 0;
}