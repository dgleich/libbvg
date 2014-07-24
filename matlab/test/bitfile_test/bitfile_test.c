/* 
 * David Gleich
 * Copyright, Stanford University, 2007
 * 17 May 2007
 */

/**
 * @file bitfile_test.c
 * Read a set of numbers from a bitfile to test the implemenation.
 */

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



int main(int argc, char **argv)
{
    FILE *f;
    bitfile bfstruct, *bf;
    int i;
    unsigned char *mem;

    f = fopen("correct-data","rb");
    bf = &bfstruct;
    bitfile_open(f, bf);
    
    printf("%"PRId64"\n", bitfile_read_int(bf,10));
    printf("%"PRId64"\n", bitfile_read_int(bf,3));
    printf("%"PRId64"\n", bitfile_read_int(bf,3));
    printf("%"PRId64"\n", bitfile_read_int(bf,16));
    printf("%"PRId64"\n", bitfile_read_int(bf,30));
    
    
    for (i = 0; i < 64; i++) 
    {
        printf("%d\n", bitfile_read_unary(bf));
    }
    
    for (i = 0; i < 64; i++) 
    {
        printf("%"PRId64"\n", bitfile_read_gamma(bf));
    }
    
    for (i = 0; i < 64; i++) 
    {
        printf("%"PRId64"\n", bitfile_read_zeta(bf,3));
    }

    bitfile_close(bf);

    fseek(f, 0, SEEK_END);
    i = ftell(f);
    mem = malloc(i);
    fseek(f, 0, SEEK_SET);
    fread(mem, 1, i, f);

    bitfile_map(mem, i, bf);

    printf("%"PRId64"\n", bitfile_read_int(bf,10));
    printf("%"PRId64"\n", bitfile_read_int(bf,3));
    printf("%"PRId64"\n", bitfile_read_int(bf,3));
    printf("%"PRId64"\n", bitfile_read_int(bf,16));
    printf("%"PRId64"\n", bitfile_read_int(bf,30));
    
    
    for (i = 0; i < 64; i++) 
    {
        printf("%d\n", bitfile_read_unary(bf));
    }
    
    for (i = 0; i < 64; i++) 
    {
        printf("%"PRId64"\n", bitfile_read_gamma(bf));
    }
    
    for (i = 0; i < 64; i++) 
    {
        printf("%"PRId64"\n", bitfile_read_zeta(bf,3));
    }

    bitfile_close(bf);
 
    return (0);
}