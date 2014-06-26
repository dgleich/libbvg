#include "bitfile.h"

#include <stdlib.h>
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
    int val32bit[5] = {13, 4, 3, 45233, 232584213};
    int num32bit = 5;
    int i, size;
    int64_t res;
    int state;
    if (!f) {
        printf("Unable to open the file!\n");
        return (-1);
    }
    state = bitfile_open(f, &bf);
    if (state) {
        printf("Error: bitfile_open failed!\n");
    }
    for (i = 0; i < num32bit; i ++) {
        res = bitfile_read_int(&bf, len32bit[i]);
        if (res != val32bit[i]) {
            printf("32 bit test failed on value %d\n", val32bit[i]);
            return (-1);
        }
    }
    size = 64;
	for (i = 0; i < size; i ++) {
        res = (int64_t)bitfile_read_unary(&bf);
        if (res != i) {
            printf("32-bit unary test failed on value %d\n", i);
            return (-1);
        }
	}
	for (i = 0; i < size; i ++) {
        res = bitfile_read_gamma(&bf);
        if (res != 7*i + i) {
            printf("32-bit gamma integer test failed on value %d\n", 7*i + i);
            return (-1);
        }
    }
	
    for (i = 0; i < size; i ++) {
        res = bitfile_read_zeta(&bf, 3);
        if (res != 7*i + i) {
            printf("32-bit zeta integer test failed on value %d\n", 7*i + i);
            return (-1);
        }
    }

    res = bitfile_read_int(&bf, 35);
    if (res != 4294967296l) {
        printf("64-bit integer read test failed on value %ld\n", 4294967296L);
        return (-1);
    }
		
    for (i = 0; i < 16; i ++) {
        res = bitfile_read_gamma(&bf);
        if (res != 4294967296l*(1L << i)) {
            printf("64-bit gamma integer test failed on value %ld\n", 4294967296l*(1L << i));
            return (-1);
        }
    }
	
    for (i = 0; i < 16; i ++) {
        res = bitfile_read_zeta(&bf, 3);
        if (res != 4294967296l*(1L << i)) {
            printf("64-bit zeta integer test failed on value %ld\n", 4294967296l*(1L << i));
            return (-1);
        }
    }
	
    bitfile_close(&bf);
    fclose(f);
    printf("bitfile 64-bit test passed.\n");
    return (0);
}

