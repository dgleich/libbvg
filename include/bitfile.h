#ifndef LIBBVG_BITFILE_H
#define LIBBVG_BITFILE_H

/*
 * David Gleich
 * Copyright, Stanford University, 2007
 * 17 May 2007
 */

/**
 * @file bitfile.h
 * The main declarations for the bitfile interface.
 */

/** History
 *
 * 2008-03-10: Added bitfile position header, skip_* functions
 */

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__GNUC__)
#include <stdint.h>
#endif

#ifndef int64
#if defined(_MSC_VER)
	typedef signed __int64 int64;
#elif defined(__GNUC__)
	typedef int64_t int64;
#endif
#endif
	
#ifndef uint64
#if defined(_MSC_VER)
	typedef unsigned __int64 uint64;
#elif defined(__GNUC__)
	typedef uint64_t uint64;
#endif	
#endif

struct bitfile_tag {
    FILE* f;
    unsigned char *buffer;  // buffer points to the current buffer
    const unsigned char *memory;  // memory points to a preloaded file
    int wrapping;    // true if wrapping a memory array
    int use_buffer; // false if we shouldn't use the buffer
    size_t bufsize; // the size of the buffer
    
    size_t pos; // the position in the byte buffer;
    size_t avail; // number of bytes available in the byte buffer
    
    size_t position; // current position of the first byte of the buffer
    
    // so the current position in the data is  
    // bf->position  (location in file for start of buffer) 
    // + bf->pos     (location in buffer)

    unsigned long long total_bits_read; // total bits read

    size_t current; // the current bit buffer
    size_t fill; // current number of bits in the bit buffer
    
    // the bits current & (2 << fill - 1) yield the next bits
    // in the file
    // where the subsequent bits are exactly:
    //   ((current & (1 << fill - 1)) >> (fill - 1)) & 1
    //   ((current & (1 << fill - 1)) >> (fill - 2)) & 1
    //   ...
    //   ((current & (1 << fill - 1)) >> (fill - fill)) & 1
    // see read_from_current in bitfile.c
};

typedef struct bitfile_tag bitfile;

int bitfile_open(FILE* f, bitfile* bf);
int bitfile_map(unsigned char* mem, size_t len, bitfile *bf);
int bitfile_close(bitfile* bf);
int bitfile_flush(bitfile* bf);

int bitfile_read_bit(bitfile* bf);
uint64 bitfile_read_int(bitfile* bf, unsigned int len);
int bitfile_read_unary(bitfile* bf);
uint64 bitfile_read_gamma(bitfile* bf);
uint64 bitfile_read_zeta(bitfile* bf, const int k);
uint64 bitfile_read_nibble(bitfile* bf);
int bitfile_check_long(uint64 x);

long long bitfile_tell(bitfile* bf);
int bitfile_position(bitfile* bf, const long long pos);

long long bitfile_skip(bitfile* bf, long long n);
int bitfile_skip_gammas(bitfile* bf, int n);
int bitfile_skip_deltas(bitfile* bf, int n);

#ifdef __cplusplus
}
#endif

#endif /* LIBBVG_BITFILE_H */

