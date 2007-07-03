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

#include <stdio.h>

struct bitfile_tag {
    FILE* f;
    unsigned char *buffer;  // buffer points to the current buffer
    const unsigned char *memory;  // memory points to a preloaded file
    int wrapping;    // true if wrapping a memory array
    int use_buffer; // false if we shouldn't use the buffer
    size_t bufsize; // the size of the buffer

    unsigned long long total_bits_read; // total bits read

    size_t current; // the size of the current bit buffer
    size_t fill; // current number of bits in the bit bufer
    size_t pos; // the position in the byte buffer;
    size_t avail; // number of bytes available in the byte buffer
    
    size_t position; // current position of the first byte of the buffer
};

typedef struct bitfile_tag bitfile;

int bitfile_open(FILE* f, bitfile* bf);
int bitfile_map(unsigned char* mem, size_t len, bitfile *bf);
int bitfile_close(bitfile* bf);
int bitfile_flush(bitfile* bf);

int bitfile_read_bit(bitfile* bf);
int bitfile_read_int(bitfile* bf, unsigned int len);
int bitfile_read_unary(bitfile* bf);
int bitfile_read_gamma(bitfile* bf);
int bitfile_read_zeta(bitfile* bf, const int k);
int bitfile_read_nibble(bitfile* bf);

#endif /* LIBBVG_BITFILE_H */

