/*
 * David Gleich
 * Copyright, Stanford University, 2007
 * 17 May 2007
 */

/**
 * @file bitfile.c
 * The implementations behind the bitfile wrapper around 
 * a file pointer and an in-memory array.
 */
 
 /** History
 *
 * 2007-07-02
 * Commented out the refill codes to prevent gcc warning
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bitfile.h"

#include <assert.h>

/**
 * Allocate a few lookup tables to speed the computations
 */
const int BYTELSB[] = {
        -1, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
};
    
const int BYTEMSB[] = {
        -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};

/**
 * Allocate a few constants to speed the algorithms.
 */
const int bitfile_default_buffer_size = 16*1024;

/**
 * Wrap a file pointer as a bitstream.  The wrapping provides
 * access to the sequential bits of the file.
 *
 * @param f the open and valid file pointer.  
 * @param bf the newly created bitfile structure.
 * @rval 0 if everything succeeded
 * 
 * @example
 * FILE *f;
 * bitfile bf;
 * int state
 * f = fopen("myfile","rb");
 * state = bitfile_open(f,&bf);
 * if (state) { // error! }
 */
int bitfile_open(FILE* f, bitfile* bf)
{
    // clear the existing bitfile
    memset(bf, 0, sizeof(bitfile));

    bf->f = f;
    bf->buffer = malloc(sizeof(unsigned char)*bitfile_default_buffer_size);
    bf->bufsize = bitfile_default_buffer_size;
    bf->use_buffer = 1;

    if (bf->buffer == NULL) {
        return -1;
    } else {
        return 0;
    }
}

/**
 * Wrap an exisiting array as a bitfile.  The result is something that acts
 * just like a bitfile, but never actually touches the underlying disk.
 *
 * @param mem the in memory array
 * @param len the length of the in memory array
 * @param bf the newly created bitfile structure
 * @return 0 if everything succeeded
 * @example
 * int mem = ; // encode the first 5 digits of pi in unary
 * bitfile bf;
 * bitfile_map((void *)&mem, 4, &bf);
 * fprintf("%i %i %i %i %i\n", 
 *   bitfile_read_unary(bf), 
 *   bitfile_read_unary(bf), 
 *   bitfile_read_unary(bf), 
 *   bitfile_read_unary(bf), 
 *   bitfile_read_unary(bf));
 */
int bitfile_map(unsigned char* mem, size_t len, bitfile *bf)
{
    // clear the existing bitfile
    memset(bf, 0, sizeof(bitfile));

    bf->buffer = mem;
    bf->wrapping = 1;
    bf->use_buffer = 1;
    bf->avail = len;
    bf->bufsize = len;

    return (0);
}

/**
 * Close a bitfile structure which will free any memory allocated for the
 * structure.
 * 
 * @param bf the bitfile
 * @return 0 on success 
 */
int bitfile_close(bitfile* bf)
{
    if (bf->use_buffer && !bf->wrapping) { free(bf->buffer); }

    return (0);
}

/**
 * Flush a bitfile, which allows the underlying stream to be repositioned.
 * @param bf the bitfile
 * @return 0 on success
 */
int bitfile_flush(bitfile* bf)
{
    if (!bf->wrapping) {
        bf->position += bf->pos;
        bf->avail = 0;
        bf->pos = 0;
    }
    bf->fill = 0;

    return (0);
}

/**
 * Read the next byte from the underlying stream but don't update total_bits_read.
 *
 * @param bf the bitfile
 * @rval the byte
 */
static int bitfile_read(bitfile* bf)
{
    assert( bf->use_buffer );

    if (bf->avail == 0) 
    {
        if (bf->f) {
            bf->avail = (int)fread(bf->buffer,1,bf->bufsize,bf->f);
        }

        if (bf->avail <= 0) { 
            return -2; 
        }
        else {
            bf->position += bf->pos;
            bf->pos = 0;
        }
    }

    bf->avail--;
    return bf->buffer[bf->pos++] & 0xFF;
}

/** Fills {@link #current} to 16 bits.
 * 
 * <p>This method must be called <em>only</em> when 8 &le; {@link #fill} &lt; 16. It
 * will try to put 8 more bits into {@link #current}, so that at least 16 bits will be valid.
 * It will not throw an {@link EOFException}&mdash;simply, {@link #current} will remain unchanged.
 * 
 * @return {@link #fill}.
 */
/*static int refill16(bitfile* bf) 
{
    assert( bf->fill >= 8 );
    assert( bf->fill < 16 );

    if (bf->avail > 0) {
        // if there is a current byte in the buffer, use it directly.
        bf->avail--;
        bf->current = (bf->current << 8) | (bf->buffer[bf->pos++] & 0xFF);
    }

    bf->current = (bf->current << 8) | bitfile_read(bf);
    return (int)(bf->fill += 8);
}
           
static int refill(bitfile* bf)
{
    if (bf->fill == 0) {
        bf->current = bitfile_read(bf);
        return (int)(bf->fill = 8);
    }
    
    if (bf->avail > 0) {
        bf->avail--;
        bf->current = (bf->current << 8) | (bf->buffer[bf->pos++] & 0xFF);
        return (int)(bf->fill += 8);
    }
    
    bf->current = (bf->current << 8) | bitfile_read(bf);
    return (int)(bf->fill += 8);
}*/

/**
 * Read bits from the buffer, possibly refilling it.
 */
static int read_from_current(bitfile *bf, const size_t len)
{
    unsigned int rval;
    if (len == 0) { return 0; }
    if (bf->fill == 0) {  bf->current = bitfile_read(bf); bf->fill = 8; }
    bf->total_bits_read += len;
    rval = (unsigned)bf->current;
    return (rval >> ( bf->fill -= len) & ((1 << len) - 1));
}
            
/**
 * Read a single bit.
 * @param bf the bitfile
 * @return the bit
 */
int bitfile_read_bit(bitfile* bf)
{
    return read_from_current(bf,1);
}

/**
 * Read a set of bits and interpret them as a non-negative integer value.
 * @param bf the bitfile
 * @param len the number of bits
 * @return the non-negative integer represented by the bits
 */
int bitfile_read_int(bitfile* bf, unsigned int len)
{
    int i, x = 0;
    assert ( len >=  0 );
    assert ( len <= 32 );

    if (len <= bf->fill) {
        return read_from_current(bf, len);
    }

    len -= (unsigned int)bf->fill;
    x = read_from_current(bf, bf->fill);
    i = len >> 3;
    while (i-- != 0) { x = x << 8 | bitfile_read(bf); }
    bf->total_bits_read += len & ~7;

    len &= 7;

    return (x << len) | read_from_current(bf, len);
}

/**
 * Read a unary value, which is a series of 0s and then terminated by
 * a one.  The value is the number of 0 bits.
 * @param bf the bitfile
 * @return the value in unary
 */
int bitfile_read_unary(bitfile* bf)
{
    int x;
    unsigned int current_left_aligned;
    assert ( bf->fill < 24 );
    current_left_aligned = (unsigned int)(bf->current << (24 - bf->fill) & 0xFFFFFF);
    if (current_left_aligned != 0)
    {
        if ((current_left_aligned & 0xFF0000) != 0) { x = 7 - BYTEMSB[current_left_aligned >> 16]; }
        else if ((current_left_aligned & 0xFF00) != 0) { x = 15 - BYTEMSB[current_left_aligned >> 8]; }
        else { x = 23 - BYTEMSB[current_left_aligned & 0xFF]; }
        bf->total_bits_read += x + 1;
        bf->fill -= x + 1;
        return (x);
    }
    
    x = (int)bf->fill;
    while ( (bf->current = bitfile_read(bf)) == 0) { x += 8; }
    x += 7 - (int)( bf->fill = BYTEMSB[bf->current] );
    bf->total_bits_read += x + 1;
    return (x);
}

/**
 * Read a gamma coded integer.
 * @param bf the bitfile
 * @return the value of the gamma coded integer
 */
int bitfile_read_gamma(bitfile* bf)
{
    const int msb = bitfile_read_unary(bf);
    return ( ( 1 << msb ) | bitfile_read_int(bf,msb) ) - 1;
}
/**
 * Read a zeta coded integer.
 * @param bf the bitfile
 * @param k the parameter k in the gamma code.
 * @return the value of the zeta(k) coded integer.
 */
int bitfile_read_zeta(bitfile* bf, const int k)
{
    const int h = bitfile_read_unary(bf);
    const int left = 1 << h * k;
    const int m = bitfile_read_int(bf,h*k + k - 1);
    if (m < left) { return (m + left - 1); }
    else { return (m << 1) + bitfile_read_bit(bf) - 1; }
}

/**
 * Read a nibbled coded integer.
 * @param bf the bitfile
 * @return the nibble coded integer.
 */
int bitfile_read_nibble(bitfile* bf)
{
    int b;
    int x = 0;
    do {
        x <<= 3;
        b = bitfile_read_bit(bf);
        x |= bitfile_read_int(bf,3);
    } while (b == 0);
    return x;
}

