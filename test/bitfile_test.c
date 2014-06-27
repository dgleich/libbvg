/* 
 * David Gleich
 * Copyright, Stanford University, 2007
 * 17 May 2007
 */

/**
 * @file bitfile_test.c
 * Read a set of numbers from a bitfile to test the implemenation.
 */

/** Updates
 *
 * 2008-03-10: Added skip tests.
 *             Added true answers.
 *             Rewrote main to into test_read.
 * 2008-03-11: Added test_position.
 */

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

int truevals[] = {
    13, 4, 3, 45233, 232584213,
      0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,
     13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,
     26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,
     39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,
     52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
      0,   8,  16,  24,  32,  40,  48,  56,  64,  72,  80,  88,  96,
    104, 112, 120, 128, 136, 144, 152, 160, 168, 176, 184, 192, 200,
    208, 216, 224, 232, 240, 248, 256, 264, 272, 280, 288, 296, 304,
    312, 320, 328, 336, 344, 352, 360, 368, 376, 384, 392, 400, 408,
    416, 424, 432, 440, 448, 456, 464, 472, 480, 488, 496, 504,
      0,   8,  16,  24,  32,  40,  48,  56,  64,  72,  80,  88,  96,
    104, 112, 120, 128, 136, 144, 152, 160, 168, 176, 184, 192, 200,
    208, 216, 224, 232, 240, 248, 256, 264, 272, 280, 288, 296, 304,
    312, 320, 328, 336, 344, 352, 360, 368, 376, 384, 392, 400, 408,
    416, 424, 432, 440, 448, 456, 464, 472, 480, 488, 496, 504,
};

int test_position();
int test_skip();
int test_read();

int main(int argc, char **argv)
{
    int rval = 0;
    printf("Testing read functions ... ");
    rval = test_read();
    if (rval != 0) {
        fprintf(stderr, "FAILED!\n");
        return (-1);
    } else {
        printf("passed!\n");
    }

    printf("Testing skip functions ... ");
    rval = test_skip();
    if (rval != 0) {
        fprintf(stderr, "FAILED!\n");
        return (-1);
    } else {
        printf("passed!\n");
    }

    printf("Testing position functions ... ");
    rval = test_position();
    if (rval != 0) {
        fprintf(stderr, "FAILED!\n");
        return (-1);
    } else {
        printf("passed!\n");
    }
 
    return (0);
}

int test_val(int vi, int v, int* truevals) {
    if (v != truevals[vi]) {
        fprintf(stderr, "\n ERROR on value %4i : %8i read from file, should be %8i\n",
            vi, v, truevals[vi]);
        return (-1);
    } else {
        return (0);
    }
}

#define CHECKVAL(vi, v, truevals) { \
    if (test_val(vi, v, truevals) != 0) { return (vi); } \
    else { vi++; } }


int test_read_1(bitfile *bf, int* truevals)
{
    int i, vi=0;

    CHECKVAL(vi, bitfile_read_int(bf,10), truevals);
    CHECKVAL(vi, bitfile_read_int(bf,3), truevals);
    CHECKVAL(vi, bitfile_read_int(bf,3), truevals);
    CHECKVAL(vi, bitfile_read_int(bf,16), truevals);
    CHECKVAL(vi, bitfile_read_int(bf,30), truevals);
    
    for (i = 0; i < 64; i++) {
        CHECKVAL(vi, bitfile_read_unary(bf), truevals);
    }
    
    for (i = 0; i < 64; i++) {
        CHECKVAL(vi, bitfile_read_gamma(bf), truevals);
    }
    
    for (i = 0; i < 64; i++) {
        CHECKVAL(vi, bitfile_read_zeta(bf,3), truevals);
    }

    return (vi);
}

/** Test the bitfile_read functions
 * @return 0 on success
 */
int test_read() 
{
    FILE *f;
    bitfile bfstruct, *bf;
    unsigned char *mem;
    int filesize, rval, ntruevals = sizeof(truevals)/sizeof(int);
   
    f = fopen("correct-data","rb");
    bf = &bfstruct;
    bitfile_open(f, bf);

    if ((rval = test_read_1(bf, truevals)) != ntruevals) {
        fprintf(stderr, "error reading data from file (%4i of %4i entries)...\n",
            rval, ntruevals);
        return (-1);
    }

    bitfile_close(bf);

    fseek(f, 0, SEEK_END);
    filesize = ftell(f);
    mem = malloc(filesize);
    fseek(f, 0, SEEK_SET);
    fread(mem, 1, filesize, f);

    bitfile_map(mem, filesize, bf);

    if ((rval = test_read_1(bf, truevals)) != ntruevals) {
        fprintf(stderr, "error reading data from memory (%4i of %4i entries)...\n",
            rval, ntruevals);
        return (-1);
    }

    bitfile_close(bf);
    fclose(f);

    return (0);
}

int test_position_1(bitfile* bf, int* truevals) 
{
    int i, off=0, vi=0;

    CHECKVAL(vi, bitfile_read_int(bf,10), truevals);
    CHECKVAL(vi, bitfile_read_int(bf,3), truevals);
    CHECKVAL(vi, bitfile_read_int(bf,3), truevals);
    CHECKVAL(vi, bitfile_read_int(bf,16), truevals);
    CHECKVAL(vi, bitfile_read_int(bf,30), truevals);
    
    for (i = 0; i < 64; i++) {
        // each unary takes (i+1) bytes, so we just always position the stream
        bitfile_position(bf, 62+off); off+=(i+1);
        CHECKVAL(vi, bitfile_read_unary(bf), truevals);
    }

    // reset the stream
    bitfile_position(bf, 0);
    vi=0;
    CHECKVAL(vi, bitfile_read_int(bf,10), truevals);
    CHECKVAL(vi, bitfile_read_int(bf,3), truevals);
    CHECKVAL(vi, bitfile_read_int(bf,3), truevals);
    CHECKVAL(vi, bitfile_read_int(bf,16), truevals);
    CHECKVAL(vi, bitfile_read_int(bf,30), truevals);
    
    // position after all the unary values
    bitfile_position(bf,62+off); 
    vi += 64;
    
    for (i = 0; i < 64; i++) {
        CHECKVAL(vi, bitfile_read_gamma(bf), truevals);
    }
    
    for (i = 0; i < 64; i++) {
        CHECKVAL(vi, bitfile_read_zeta(bf,3), truevals);
    }

    return (vi);
}

/** Test the bitfile_position function
 * @return 0 on success
 */
int test_position()
{
    FILE *f;
    bitfile bfstruct, *bf;
    unsigned char *mem;
    int filesize, rval, ntruevals = sizeof(truevals)/sizeof(int);

    f = fopen("correct-data","rb");
    bf = &bfstruct;
    bitfile_open(f, bf);

    if ((rval = test_position_1(bf, truevals)) != ntruevals) {
        fprintf(stderr, "error with position function from file (%4i of %4i entries)...\n",
            rval, ntruevals);
        return (-1);
    }

    bitfile_close(bf);

    fseek(f, 0, SEEK_END);
    filesize = ftell(f);
    mem = malloc(filesize);
    fseek(f, 0, SEEK_SET);
    fread(mem, 1, filesize, f);

    bitfile_map(mem, filesize, bf);

    if ((rval = test_position_1(bf, truevals)) != ntruevals) {
        fprintf(stderr, "error with position function from memory (%4i of %4i entries)...\n",
            rval, ntruevals);
        return (-1);
    }
    
    bitfile_close(bf);
    fclose(f);

    return (0);
}

int test_skip_1(bitfile* bf, int* truevals)
{
    int i, vi=0;
    // instead of
    //CHECKVAL(vi, bitfile_read_int(bf,10), truevals);
    //CHECKVAL(vi, bitfile_read_int(bf,3), truevals);
    //CHECKVAL(vi, bitfile_read_int(bf,3), truevals);
    bitfile_skip(bf, 16); vi += 3;
    CHECKVAL(vi, bitfile_read_int(bf,16), truevals);
    //CHECKVAL(vi, bitfile_read_int(bf,30), truevals);
    bitfile_skip(bf, 30); vi += 1;
    
    for (i = 0; i < 64; i++) {
        CHECKVAL(vi, bitfile_read_unary(bf), truevals);
    }
    
    for (i = 0; i < 64; i++) {
        int n = 1, v;
        if ((i+n)<(64-1)) { bitfile_skip_gammas(bf,n); i+=n; vi+=n; }
        v = bitfile_read_gamma(bf); // printf("vi = %4i; v = %5i\n", vi, v);
        CHECKVAL(vi, v, truevals);
    }
    
    for (i = 0; i < 64; i++) {
        CHECKVAL(vi, bitfile_read_zeta(bf,3), truevals);
    }

    return (vi);
}


/** Test the bitfile_skip_* functions
 * @return 0 on success
 */
int test_skip()
{
    FILE *f;
    bitfile bfstruct, *bf;
    unsigned char *mem;
    int filesize, rval, ntruevals = sizeof(truevals)/sizeof(int);

    f = fopen("correct-data","rb");
    bf = &bfstruct;
    bitfile_open(f, bf);

    if ((rval = test_skip_1(bf, truevals)) != ntruevals) {
        fprintf(stderr, "error skipping over data from file (%4i of %4i entries)...\n",
            rval, ntruevals);
        return (-1);
    }

    bitfile_close(bf);

    fseek(f, 0, SEEK_END);
    filesize = ftell(f);
    mem = malloc(filesize);
    fseek(f, 0, SEEK_SET);
    fread(mem, 1, filesize, f);

    bitfile_map(mem, filesize, bf);

    if ((rval = test_skip_1(bf, truevals)) != ntruevals) {
        fprintf(stderr, "error skipping over data from memory (%4i of %4i entries)...\n",
            rval, ntruevals);
        return (-1);
    }
    
    bitfile_close(bf);
    fclose(f);

    return (0);
}
