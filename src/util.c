/* 
 * David Gleich
 * Copyright, Stanford University, 2007
 * 17 May 2007
 */

/**
 * @file util.c
 * Include a set of utility routines to make a few operations a little easier.
 */

/** History
 *
 *  2008-01-15: Added __APPLE_CC__ case to fsize to get correct size 
 *              in that case.
 *  2008-05-09: Added int_vector_create_copy
 *              Fixed int_vector_ensure_size to remove spurious alloc on
 *              >= n instead of > n
 */

#ifdef __GNUC__
#define _FILE_OFFSET_BITS 64
#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE
#endif /* __GNUC__ */

#include "bvgraph_internal.h"

#ifdef __GNUC__
#include <unistd.h>
#endif /* __GNUC__ */

#include <sys/types.h> 
#include <sys/stat.h> 


/**
 * Convert a specified number of characters into a number.  This
 * function is a more secure version of atoi, because it does
 * not depend on null-termination.
 *
 * @param str the string with a digit
 * @param len the length of the string
 * @return the value encoded by the string
 */
int atoin(const char* str, uint len)
{
    int value = 0;
    int sign = 1;
    int digit = 0;
    uint i = 0;

    const char *p = str;
    // skip to the end of the whitespace (incrementing the position and the index)
    while (i < len && *p && (*p == ' ' || *p == '\t' || *p == '\f')) { p++; i++; }

    if (i < len && *p) {
        if (*p == '-') { sign = -1; p++; i++; }
    }
    while (i < len && *p) {
        digit = *p - '0';
        if (digit >= 0 && digit <= 9) {
            value = value*10 + digit;
        } else {
            break;
        }
        *p++; i++;
    }

    return (value*sign);
}

/*
 * append two strings of fixed length, and return the result 
 * as a new string.  the CALLER IS RESPONSIBLE FOR FREEING the
 * new string.
 */
char* strappend(const char* str, uint len, const char* str2, uint len2)
{
    char *rval = malloc(len+len2+1);
    if (!rval) { return NULL; }

    strncpy(rval,str,len);
    strncpy(rval+len,str2,len2);
    rval[len+len2] = '\0';

    return rval;
}

/** Search for a character in a length limited string
 * 
 * strchrn returns a pointer to the first occurence of 
 * the c in str 
 *
 * @param str the string
 * @param len the length of the string
 * @param c the character
 * @return the pointer to the first occurrence of c in str or NULL
 * if c does not occur in the first len positions of str.
 */
const char* strchrn(const char* str, uint len, int c)
{
    uint pos = 0;
    while (pos < len) {
        if (*str == c) { return str; }
        *str++; pos++;
    }
    return (NULL);
}

/**
 * Advance the file pointer to the start of the next line
 *
 * The file must be opened as a text file for this routine to 
 * work so that newlines are translated to '\n' and not
 * flagged as '\r' and then '\n';
 */
void fnextline(FILE *f)
{
    while (!feof(f)) {
        int c = getc(f);
        if (c == '\n' || c == '\r') { break; }
    }
}

/**
 * Skip characters from the set of characters
 * in the string schars.
 *
 * On return, the file pointer will be positioned at the first
 * non-skipped character, or the end of the file
 *
 * @param f the file pointer
 * @param schars a string of characters to skip
 * @param scharslen the number of characters characters 
 */
void fskipchars(FILE *f, const char *schars, uint scharslen)
{
    uint i;
    int c;
    while (!feof(f)) {
        c = getc(f);
        for (i=0; i < scharslen; i++) {
            if (c == schars[i]) { break; }
        }
        // i == whitelen when the loop exited without a break
        if (i == scharslen) { ungetc(c, f); break; }
    }
}

int ftestnewline(FILE *f)
{
    int c = getc(f);
    if (c == '\n' || c == '\r') { return 1; }
    else { ungetc(c, f); return 0; }
}

/**
 * This function is a pseudo-portable file size function in C
 * that works for the two compilers I intend to use for this code
 * namely gcc and msvc.
 *
 * @param filename the name of the file 
 * @param s the size of the file
 * @return 0 on success
 */
int fsize(const char *filename, unsigned long long *s)
{
#ifdef _MSC_VER
    struct __stat64 file_stat; 
    int err = _stat64( filename, &file_stat ); 
    if (err) { return -1; }
    *s = file_stat.st_size;
#elif defined(__APPLE_CC__)
    struct stat file_stat;
    int err = stat(filename, &file_stat );
    if (err) { return -1; }
    *s = (unsigned long long)file_stat.st_size;
#elif defined __GNUC__
    struct stat64 file_stat;
    int err = stat64(filename, &file_stat );
    if (err) { return -1; }
    *s = file_stat.st_size;
#endif
    return 0;
}

int int_vector_create(bvgraph_int_vector* v, uint n)
{
    v->elements = n;
    v->a = malloc(sizeof(int)*n);
    if (!v->a) { return bvgraph_call_out_of_memory; }
    return (0);
}

/** Create a new int_vector from a deep copy of another int_vector.
 * 
 * @param u the new vector
 * @param v the old vector
 */
int int_vector_create_copy(bvgraph_int_vector* u, bvgraph_int_vector *v)
{
    u->elements = v->elements;
    u->a = malloc(sizeof(int)*u->elements);
    if (!u->a) { return bvgraph_call_out_of_memory; }
    memcpy(u->a, v->a, sizeof(int)*u->elements);
    return (0);
}

int int_vector_ensure_size(bvgraph_int_vector *v, uint n)
{
    if (n > v->elements) {
        int* newa = malloc(sizeof(int)*n);
        if (!newa) { return bvgraph_call_out_of_memory; }

        memcpy(newa, v->a, sizeof(int)*v->elements);
        free(v->a);
        v->a = newa;
        v->elements = n;
    }
    return (0);
}
int int_vector_free(bvgraph_int_vector* v)
{
    if (v->a) { free(v->a); v->a = NULL; }

    return (0);
}

