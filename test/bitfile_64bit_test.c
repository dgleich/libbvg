#include "bitfile.h"

#include <stdlib.h>
#include <inttypes.h>  
// disable all of the unsafe operation warnings
#ifdef _MSC_VER
#define inline __inline
#if _MSC_VER >= 1400
#pragma warning ( push )
#pragma warning ( disable: 4996 )
#endif /* _MSC_VER >= 1400 */
#endif /* _MSC_VER */

// this main function test the datafile "correct-data-64"
int main(int argc, char **argv)
{
    FILE *f = fopen("./correct-data-64", "rb");
    bitfile bf;
    int len32bit[5] = {10, 3, 3, 16, 30};
    int num32bit = 5;
    int i, size;
    int64_t res;
    int state;
    if (!f) {
        printf("Unable to open the file!\n");
        return (1);
    }
    printf("Testing read 32-bit integers ... \n");
    state = bitfile_open(f, &bf);
    if (state) {
        printf("Error: bitfile_open failed!\n");
    }
    for (i = 0; i < num32bit; i ++) {
        res = bitfile_read_int(&bf, len32bit[i]);
        printf("%" PRId64 "\n", res);
    }
    size = 64;
    printf("Testing read unary integers (0 - 63)) \n");
	for (i = 0; i < size; i ++) {
        res = (int64_t)bitfile_read_unary(&bf);
        printf("%" PRId64 "\n", res);
	}
    printf("Testing read 32-bit gamma integer ... \n");
	for (i = 0; i < size; i ++) {
        res = bitfile_read_gamma(&bf);
        printf("%d -- %" PRId64 "\n", 7*i+i,res);
    }
	
    printf("Testing read 32-bit zeta integer ... \n");
    for (i = 0; i < size; i ++) {
        res = bitfile_read_zeta(&bf, 3);
        printf("%d -- %" PRId64 "\n", 7*i+i, res);
    }

    printf("Testing read 64-bit long integer ...\n");
    res = bitfile_read_int(&bf, 35);
    printf("%" PRId64 "\n", res);
	
    printf("Testing read 64-bit gamma integer ...\n");	
    for (i = 0; i < 16; i ++) {
        res = bitfile_read_gamma(&bf);
        printf("%ld -- %" PRId64 "\n", 4294967296l*(1L << i), res);
    }
	
    printf("Testing read 64-bit zeta integer ...\n");
    for (i = 0; i < 16; i ++) {
        res = bitfile_read_zeta(&bf, 3);
        printf("%ld -- %" PRId64 "\n", 4294967296l*(1L << i), res);
    }
	
    bitfile_close(&bf);
    fclose(f);
 
    return (0);
}

