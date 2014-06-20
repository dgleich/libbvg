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
 * 2007-07-02: Commented out the refill codes to prevent gcc warning
 * 2008-03-10: Added skip_bytes section for portable byte skips
 *             Added bitfile_skip_gammas and bitfile_skip_deltas
 *             Added bitfile_position
 *             Added bitfile_skip
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bitfile.h"

#include <assert.h>
#include "gamma.h"
#include "zeta.h"

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

// pre-computed table for gamma encoding
//int GAMMA[256*256];

/** Define a portable seek function that returns the number of bytes moved
 * 
 * This function is implemented slightly differently on each architecture,
 * but functions like a java "skip" command.
 *
 * If the size of the file is less than the number of bytes, this returns
 * less than n. 
 * 
 * @param f the file stream to skip n bytes
 * @param n the number of bytes to skip.
 * @return the number of bytes skipped, or 
 *         negative values to indicator an error
 */
long long skip_bytes(FILE *f, long long n)
{
#if defined(_MSC_VER)
    long long start, end;
    int rval;
    start = _ftelli64(f);
    rval = _fseeki64(f, n, SEEK_CUR);
    if (rval == -1) { return -1; }
    end = _ftelli64(f);
    return (end-start);
#elif defined(__GNUC__)
    off_t start,end;
    int rval;
    start=ftello(f);
    rval=fseeko(f, (off_t)n, SEEK_CUR);
    if (rval == -1) { return -1; }
    end=ftello(f);
    return ((long long)(end-start));
#else
#error Please implement this function for your platform.
#endif
}

/** Position a stream at a particular byte
 */
int position_stream(FILE *f, unsigned long long n) {
#if defined(_MSC_VER)
    return _fseeki64(f, n, SEEK_SET);
#elif defined(__GNUC__)
    return fseeko(f, n, SEEK_SET);
#else
#error Please implement this function for your platform.
#endif
}
 

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
        } else {
            bf->position += bf->pos;
            bf->pos = 0;
        }
    }

    bf->avail--;
    return bf->buffer[bf->pos++] & 0xFF;
}


/** Fills the bit-buffer to at least 16-bits
 *
 * This method will ensure that bf->current has 16-bits of data,
 * *if possible* given the current state of the file.  It will
 * return the size of the fill
 * 
 * @param bf the bitfile
 * @return the current value of fill.
 */
 
// TODO, check the logistics of this with the bvgraph codes
static int refill16(bitfile *bf) {
    if (bf->fill < 16) { // make sure there is work to do
        if (bf->avail >= 2) {
            bf->current = (bf->current << 8) | (bf->buffer[bf->pos++] & 0xFF);
            bf->current = (bf->current << 8) | (bf->buffer[bf->pos++] & 0xFF);
            bf->avail -= 2;
            bf->fill += 16;
        } else {
            int byte = bitfile_read(bf);
            if (byte >= 0) { // check for error in read
                bf->current = (bf->current << 8) | byte;
                bf->fill += 8;
                byte = bitfile_read(bf);
                if (byte >= 0) {
                    bf->current = (bf->current << 8) | byte;
                    bf->fill += 8;
                }
            } 
        }
    }
        
    return bf->fill;
}

/*
static int refill16(bitfile* bf) 
{
    //assert( bf->fill >= 8 );
    assert( bf->fill < 16 );

    if (bf->avail > 1) {
        // if there is a current byte in the buffer, use it directly.
        bf->avail -= 2;
        bf->current = (bf->current << 16) | (bf->buffer[bf->pos++] & 0xFF) << 8 | (bf->buffer[bf->pos++] & 0xFF);
        return (int)(bf->fill += 16);
    }

    int read = bitfile_read(bf);
    printf("read: %d\n", read);
    bf->current = (bf->current << 8) | read; //bitfile_read(bf);
    bf->fill += 8;
    read = bitfile_read(bf);
    printf("read: %d\n", read);
    bf->current = (bf->current << 8) | read; //bitfile_read(bf);
    bf->fill += 8;
    return bf->fill;
}
*/

/*
static int refill16(bitfile* bf) 
{
    //printf("bf->fill: %d\n", bf->fill);
    //assert( bf->fill >= 8 );
    assert( bf->fill < 16 );

    printf("bf->avail: %d\n", bf->avail);

    if (bf->avail > 0) {
        // if there is a current byte in the buffer, use it directly.
        bf->avail--;
        bf->current = (bf->current << 8) | (bf->buffer[bf->pos++] & 0xFF);
    }

    int read = bitfile_read(bf);
    printf("[refill16] read: %d\n", read);
    printf("bf->avail: %d\n", bf->avail);
    if (read == -2) {
        return bf->fill;
    }

    bf->current = (bf->current << 8) | read; //bitfile_read(bf);
    return (int)(bf->fill += 8);
}
*/

/*        
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
}
*/


/** Positions the stream at a particular bit.
 *
 * <P>Given an underlying stream that implements {@link
 * RepositionableStream} or that can provide a {@link
 * java.nio.channels.FileChannel} via the <code>getChannel()</code> method,
 * a call to this method has the same semantics of a {@link #flush()},
 * followed by a call to {@link
 * java.nio.channels.FileChannel#position(long) position(position / 8)} on
 * the byte stream, followed by a {@link #skip(long) skip(position % 8)}.
 *
 * @param position the new position expressed as a bit offset.
 */
int bitfile_position(bitfile* bf, const long long position)
{
    const long long bit_delta = ((bf->position + bf->pos) << 3) - position;
    if (bit_delta >= 0 && bit_delta <= (long long)bf->fill) {
        // in this case, we have all the data loaded into the current byte
        // so we will just update our position in the byte.
        bf->fill = (int)bit_delta;
        return (0);
    } else {
        // position the byte
        const long long delta = (position >> 3) - (bf->position + bf->pos);
        const int residual = (int)(position & 7);
        // TODO check for optimizations on the long long/size_t conversion

        if (delta <= (long long)bf->avail && delta >= -(long long)bf->pos) {
            // in this case, everything is in the buffer
            bf->avail -= (size_t)delta;
            bf->pos += (size_t)delta;
            bf->fill = 0;
        } else {
            bitfile_flush(bf);
            bf->position = (size_t)(position>>3);
            //fseek(bf->f, bf->position, SEEK_SET);
            position_stream(bf->f, bf->position);   //wrong!
        }

        if (residual != 0) {
            bf->current = bitfile_read(bf);
            bf->fill = 8 - residual;
        }

        return (0);
    }
}

/** Return the current position
 * @return the number of bits current into the bitfile
 */
long long bitfile_tell(bitfile *bf)
{
    return bf->total_bits_read;
} 

/** Skips the given number of bits. 
 *
 * @param n the number of bits to skip.
 * @return @c on success, the number of bits skipped, 
 *         @c on failure, a negative number
 */
long long bitfile_skip(bitfile* bf, long long n)
{
    // handle the case when everything is in memory
    assert(n >= 0);
    if ((unsigned long long)n <= bf->fill) {
        if (n < 0) { return (-1); } // TODO give a real error code
        bf->fill -= n;
        bf->total_bits_read += n;
        return (n);
    }
    else {
        const long long prev_read_bits = bf->total_bits_read;
        // number of bytes to read
        unsigned long long nb; 
        int residual;
        n -= bf->fill;
        bf->total_bits_read += bf->fill;
        bf->fill = 0;
        
        assert(n >= 0);
        nb = n >> 3;
  
        if (bf->buffer != NULL && nb > bf->avail && nb < (bf->avail + bf->bufsize)) {
            // try loading the next byte, just in case it happens
            // to be the right one
            bf->total_bits_read += (bf->avail + 1) << 3;
            n -= (bf->avail + 1) << 3;
            nb -= bf->avail + 1;
            bf->position += bf->pos + bf->avail;
            bf->pos = bf->avail = 0;
            bitfile_read(bf);
        }

        if (nb <= bf->avail) {
            // skip directly inside the buffer
            bf->pos += (int)nb;
            bf->avail -= (int)nb;
            bf->total_bits_read += (n & ~7);
        } else {
            // this code has many warnings in the InputBitStream.java file
            // they are reproduced here

            long long to_skip;
            long long skipped;
            
            // No way, we have to pass the byte skip to the underlying stream.
            n -= bf->avail << 3;
            bf->total_bits_read += bf->avail << 3;
            
            to_skip = nb - bf->avail;
            skipped = skip_bytes(bf->f, to_skip);
            if (skipped < to_skip) { return -1; } // TODO give real error code
            bf->position += (bf->avail + bf->pos) + skipped;
            bf->pos = 0;
            bf->avail = 0;
            bf->total_bits_read += skipped << 3;

            // why exit here, we skipped too much?
            if (skipped != to_skip) { 
                return (bf->total_bits_read - prev_read_bits); 
            }
        }

        residual=(int)(n & 7);
        if (residual != 0) {
            bf->current = bitfile_read(bf);
            bf->fill = 8 - residual;
            bf->total_bits_read += residual;
        }
        return (bf->total_bits_read - prev_read_bits);
    }
}

/** Skips a given number of &gamma;-coded integers.
 *
 * <p>This method should be significantly quicker than iterating <code>n</code> times on
 * {@link #readGamma()} or {@link #readLongGamma()}, as precomputed tables are used directly,
 * so the number of method calls is greatly reduced, and the result is discarded, so
 * {@link #skip(long)} can be invoked instead of more specific decoding methods.
 * 
 * @param n the number of &gamma;-coded integers to be skipped.
 * @see #readGamma()
 */
int bitfile_skip_gammas(bitfile* bf, int n)
{
    int pre_comp;
    while (n-- != 0) {
        // don't use precomputed tables for now
        if ((bf->fill >= 16 || refill16(bf) >= 16) && 
            (pre_comp=GAMMA[bf->current>>(bf->fill-16)&0xFFFF] >> 16) != 0) {
            bf->total_bits_read += pre_comp;
            bf->fill -= pre_comp;
            continue;
        }
        bitfile_skip(bf, (long long)bitfile_read_unary(bf));
        //bitfile_read_gamma(bf);
    }
    return (0);
}

/** Skips a given number of &delta;-coded integers.
 * 
 * <p>This method should be significantly quicker than iterating <code>n</code> times on
 * {@link #readDelta()} or {@link #readLongDelta()}, as precomputed tables are used directly,
 * so the number of method calls is greatly reduced, and the result is discarded, so
 * {@link #skip(long)} can be invoked instead of more specific decoding methods.
 *
 * @param bf the bitfile 
 * @param n the number of &delta;-coded integers to be skipped.
 * @return 0 on success
 * @see #readDelta()
 */
int bitfile_skip_deltas(bitfile* bf, int n)
{
    //int pre_comp;
    while (n-- != 0) {
        // don't use precomputed tables for now
        //if ((bf->fill >= 16 || refill(bf) >= 16) && 
        //    (pre_comp = DELTA[bf->current>>(bf->fill-16)&0xFFFF]>>16) != 0) {
        //    bf->total_bits_read += pre_comp;
        //    bf->fill -= pre_comp;
        //    continue;
        //}
        bitfile_skip(bf, (long long)bitfile_read_gamma(bf));
    }

	return 0;
}
    

/**
 * Read bits from the buffer, possibly refilling it.
 */
static int read_from_current(bitfile *bf, const size_t len)
{
    unsigned int rval;
    if (len == 0) { return 0; }
    if (bf->fill == 0) {  
		bf->current = bitfile_read(bf); 
		bf->fill = 8; 
	}
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
int64 bitfile_read_int(bitfile* bf, unsigned int len)
{
    int i;
    int64 x = 0;
    assert ( len <= 64 );
    if (bf->fill < 16) { refill16(bf); }
    //printf("bf->fill: %d\n", bf->fill);

    if (len <= bf->fill) {
        return (int64)read_from_current(bf, len);
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
 * Test a 64-bit integer and see if the high 4 bytes are 0s.
 * @param x the 64-bit integer
 * @return 0 -- high 4 bytes are all 0s; 1 -- high 4 bytes are not all 0s
 *  (0 -- cannot be cast to int; 1 -- can be cast to int) 
 */
int bitfile_check_long(uint64 x)
{
    unsigned int high = 0;
    high = (x & 0xffffffff00000000) >> 32;
    if (high == 0) { return 0; }
    else { return 1; }
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
    assert ( bf->fill < 32 );
    if (bf->fill < 16) {
        //printf("refill [%d]\n", (int)bf->fill);
        refill16(bf);
        //printf("done [%d]\n", (int)bf->fill);
    }

    current_left_aligned = (unsigned int)(bf->current << (32 - bf->fill) & 0xFFFFFFFF);


    if (current_left_aligned != 0)
    {
        if ((current_left_aligned & 0xFF000000) != 0)
            { x = 7 - BYTEMSB[current_left_aligned >> 24]; }
        else if ((current_left_aligned & 0xFF0000) != 0) 
            { x = 15 - BYTEMSB[current_left_aligned >> 16]; }
        else if ((current_left_aligned & 0xFF00) != 0) 
            { x = 23 - BYTEMSB[current_left_aligned >> 8]; }
        else 
            { x = 31 - BYTEMSB[current_left_aligned & 0xFF]; }
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
int64 bitfile_read_gamma(bitfile* bf)
{
    int precomp;
    if ( ( bf->fill >= 16 || refill16(bf) >= 16 ) &&
         ( (precomp = GAMMA[ bf->current >> ( bf->fill - 16 ) & 0xFFFF ]) != 0 ) ) {
	    bf->total_bits_read += precomp >> 16;
        bf->fill -= precomp >> 16;
        return (int64)( precomp & 0xFFFF );
    }
    const int msb = bitfile_read_unary(bf);
	return ( ( 1ULL << msb ) | bitfile_read_int(bf, msb) ) - 1;
}
/**
 * Read a zeta coded integer.
 * @param bf the bitfile
 * @param k the parameter k in the gamma code.
 * @return the value of the zeta(k) coded integer.
 */
int64 bitfile_read_zeta(bitfile* bf, const int k)
{
    if ( k == 3 ) {
        int precomp;
        if ( ( bf->fill >= 16 || refill16(bf) >= 16 ) &&
             ( precomp = ZETA[ bf->current >> ( bf->fill - 16 ) & 0xFFFF ] ) != 0 ) {
            bf->total_bits_read += precomp >> 16;
            bf->fill -= precomp >> 16;
            return (int64)( precomp & 0xFFFF );
        }

    }
    
    const int h = bitfile_read_unary(bf);
    const int64 left = 1ULL << h * k;
    const int64 m = bitfile_read_int(bf,h*k + k - 1);
    if (m < left) { return (m + left - 1); }
    else { return (m << 1) + bitfile_read_bit(bf) - 1; }
}

/**
 * Read a nibbled coded integer.
 * @param bf the bitfile
 * @return the nibble coded integer.
 */
int64 bitfile_read_nibble(bitfile* bf)
{
    int b;
    int64 x = 0ULL;
    do {
        x <<= 3;
        b = bitfile_read_bit(bf);
        x |= bitfile_read_int(bf,3);
    } while (b == 0);
    return x;
}

