/* 
 * David Gleich
 * Copyright, Stanford University, 2007
 * 17 May 2007
 */

/**
 * @file properties.c
 * This is a set of routines to manage the properties file 
 * and parse a java property file in c.
 */

/** History
 *
 * 2007-05-30
 * Implemented parse_compression_flags
 * Updated parse_properties to use correct length of 
 *   each key-value pair.
 * 
 * 2007-06-10
 * Added stddef.h header for linux
 * 
 * 2007-07-02
 * Fixed error on strnlen for gcc
 * 
 * 2007-09-01
 * Added strnlen for MSVC before 2005
 *
 * 2008-01-15
 * Added strnlen for APPLE_CC 
 */
 
#ifdef __GNUC__
#define _GNU_SOURCE
#endif /* __GNUC__ */

#include "bvgraph_internal.h"
#include <stddef.h>

#if defined(_MSC_VER) && (_MSC_VER < 1400) || defined(__APPLE_CC__)
size_t strnlen(const char *s, size_t l)
{
    size_t r=0;
    while (*s++ != '\0' && l-- > 0) { r++; }
    return r;
}
#endif /* _MSC_VER < 1400 */

/**
 * This string is used as a return value from a function that
 * returns a string to indicate a parsing error, as opposed
 * to a string allocation error (which involves returning NULL);
 */
char return_error_string[] = "error string";

/**
 * A property key is any set of characters 
 *
 * @param f the property file
 * @param maxproplen the maximum length of a property
 * @return a string for the property 
 * (the CALLER is responsible for freeing the memory)
 */
char* parse_property_key(FILE *f, uint maxproplen)
{
    uint curlen = 0;
    int maybe_escape = 0;
    char *rval = NULL;
    int c;

    // check to make sure we have a valid property file
    // the entry here must not be a whitespace, ':' or "="
    c = getc(f);
    if (c == ' ' || c == '\t' || c == '\f' 
        || c == '\n' || c == '\r' 
        || c == ':' || c == '=')
    {
        ungetc(c,f);
        return return_error_string;
    }

    rval = malloc(sizeof(char)*(maxproplen+1));
    curlen = 0;
    maybe_escape = 0;
    if (!rval) { return NULL; }
    if (curlen < maxproplen) { rval[curlen++] = c; }

    while (!feof(f)) {
        c = getc(f);

        // if we see any whitespace, we are done processing
        if (!maybe_escape && 
            (c == ' ' || c == '\t' || c == '\f' || 
             c == '\n' || c == '\r' || c == '=' || c == ':')) {
            ungetc(c,f);
            break;
        }
        else if (maybe_escape) {
            // valid escape characters are 
            // \, =, :, r, n, f, t
            if (c == '\\') { 
            } else if (c == '=') {
            } else if (c == ':') {
            } else if (c == 'r') { c = '\r'; 
            } else if (c == 'n') { c = '\n';
            } else if (c == 'f') { c = '\f';
            } else if (c == 't') { c = '\t';
            } else {
                // this is an unknown escape character,
                // so toss the error string, after 
                // releasing the rval memory
                free(rval);
                return return_error_string;
            }
            maybe_escape = 0;
        }
        else if (!maybe_escape) {
            if (c == '\\') { maybe_escape = 1; continue; }
        }

        if (curlen < maxproplen) { rval[curlen++] = c; }
    }

    rval[curlen] = '\0';

    return (rval);
}




/**
 * This function must be called immediately following 
 * parse_property_key to correctly parse the property value.
 *
 * Following a property key, a property value begins
 * after any whitespace including a single '=' or ':' 
 * character
 *
 * A property value continues until the end of line 
 * character or a non-escaped '\' character.  
 * At which point parsing continues on the first non-whitespace
 * character of the next line.
 *
 * For a value of length larger than maxvallen, only the first
 * maxvallen characters are returned.  The output is always 
 * null-terminated.
 *
 * @return NULL indicates that an allocation error occurred,
 * return_error_string indicates that the file is not a valid
 * java property file, and any other return value is a valid
 * string that the CALLER MUST FREE!
 */
char* parse_property_value(FILE *f, uint maxvallen)
{
    int c;
    size_t pos1, pos2;
    uint curlen = 0;
    char *rval = NULL;

    int maybe_escape = 0;

    pos1 = ftell(f);
    fskipchars(f, " \t\f", 3);
    pos2 = ftell(f);
    
    c = getc(f);
    if (c != ':' && c != '=' && pos2 <= pos1) {
        // this case indicates that we did not 
        // aquire any whitespace and we didn't
        // see the ':' or '=' key
        if (feof(f) || c == '\n' || c == '\r') {
            // we are at the end of the
            // file or end of line, 
            // so return an empty string as the 
            // correct key
            char *rval = malloc(1);
            if (!rval) { return (NULL); }
            rval[0] = '\0';
            return rval;
        }
        else {
            // this case means that the file is
            // not a valid property file or that
            // parse_property_key was not 
            // called before this value
            return (return_error_string); 
        }
    }
    else if (c == ':' || c == '=') {
        // in this case, we saw one of the valid separating characters
        // so we just read the rest of the whitespace
        fskipchars(f," \t\f", 3);
    }
    else {
        // in the "else" case, pos2 > pos1, which just means we read whitespace
        // and indicates that we should parse the next non-whitespace variable
        // as the value, which was just stole a character from
        ungetc(c, f);
    }
        
    // allocate the output
    rval = malloc(sizeof(char)*(maxvallen+1));
    if (!rval) { return (NULL); }
    curlen = 0;

    // the maybe_escape flag is used to denote that
    // we saw a '\' character and that the next character MIGHT
    // be another '\' in which case we should count it as an escape
    // character.  If 
    maybe_escape = 0;

    while (!feof(f)) {
        c = getc(f);
        if (maybe_escape && c != '\\') {
            // TODO Check if this is the correct behavior, the documentation
            // is ambiguous because it doesn't say what to do with a odd
            // number of backslashes followed by something other than 
            // a line terminator.  In this case, the code assumes that
            // anything after an odd number of slashes is to be ignored.

            // this indicates that a logical line was split 
            // between many natural lines because we 
            // hit an odd number of '\' characters.  In this case,
            // we remove the previous character, advanced to the 
            // next line, and skip any whitespace before 
            // continuing the processing
            maybe_escape = 0;
            ungetc(c,f);
            fnextline(f);
            fskipchars(f," \t\f",3);
            continue;
        } 
        if (c == '\n' || c == '\r') {
            // we hit the end of a string!
            break;
        }
        // if we see a '\', then flip the state of maybe_escape, so if 
        // it was true, then it is now false because we saw a "\\" pair
        // if it was false, then it is now true, and we continue to 
        // see what happens for the next character
        if (c=='\\') { maybe_escape = !maybe_escape; }
        if (maybe_escape) { continue; }

        // add the character to the string
        if (curlen < maxvallen) { rval[curlen++] = c; }
    }

    // this call is always valid because rval was one
    // larger than required and we stopped at maxvallen
    rval[curlen] = '\0';

    return (rval);
}

/** Find a character in a string based on a length and position
 * 
 * Searches through the string str with length len starting
 * from the internal pointer pos.  
 *
 * @param str the string
 * @param len the length of str
 * @param pos a valid pointer into str 
 * @param c the character
 */
const char* strchrnp(const char* str, uint len, const char* pos, int c)
{
    ptrdiff_t off = pos - str;
    if (off < 0 || off >= (ptrdiff_t)len) { return (NULL); }
    return strchrn(pos, len-(uint)off, c);
}

/** Implement a small min helper function to aid cross-platform compiles
 */
size_t minf(ptrdiff_t a, size_t b) { return a<(ptrdiff_t)b?(size_t)a:b; }

/**
 * Parse the compression flags argument of the property file.
 * 
 * The compression flags are stored in a string and split by 
 * | characters.  For example, 
 *
 * "| OUTDEGREES_GAMMA | BLOCKS_GAMMA | RESIDUALS_ZETA"
 *
 * is a valid compression flag string.  
 *
 * To parse the compression flags, we split the string based on 
 * the "|" characters and then match the internal strings.
 *
 * @param g the graph structure for the flag
 * @param flagstr the string encoding the flags
 * @param len the length of the string
 * @return 0 on success
 */
int parse_compression_flags(bvgraph* g, const char* flagstr, uint len)
{
    const char* prev_bar_pos = flagstr;
    const char* next_bar_pos = flagstr;

    // begin with the default options
    g->outdegree_coding = BVGRAPH_FLAG_GAMMA;
    g->block_coding = BVGRAPH_FLAG_GAMMA;
    g->reference_coding = BVGRAPH_FLAG_UNARY;
    g->residual_coding = BVGRAPH_FLAG_ZETA;
    g->block_count_coding = BVGRAPH_FLAG_GAMMA;
    g->offset_coding = BVGRAPH_FLAG_GAMMA;

    // now parse through flagstr
    // 
    prev_bar_pos = strchrnp(flagstr, len, prev_bar_pos, '|');
    while (prev_bar_pos != NULL) {
        ptrdiff_t substrlen;
        next_bar_pos = strchrnp(flagstr, len, prev_bar_pos+1, '|');
        if (next_bar_pos == NULL) { 
            substrlen = len - (prev_bar_pos - flagstr) - 1;
        } else {
            substrlen = next_bar_pos - prev_bar_pos - 1;
        }
        if (substrlen > 0) {
                   if (strncmp(prev_bar_pos+1, " OUTDEGREES_GAMMA", minf(substrlen, 17)) == 0) {
                g->outdegree_coding = BVGRAPH_FLAG_GAMMA;
            } else if (strncmp(prev_bar_pos+1, " OUTDEGREES_DELTA", minf(substrlen, 17)) == 0) {
                g->outdegree_coding = BVGRAPH_FLAG_DELTA;
            } else if (strncmp(prev_bar_pos+1, " BLOCKS_GAMMA",     minf(substrlen, 13)) == 0) {
                g->block_coding = BVGRAPH_FLAG_GAMMA;
            } else if (strncmp(prev_bar_pos+1, " BLOCKS_DELTA",     minf(substrlen, 13)) == 0) {
                g->block_coding = BVGRAPH_FLAG_DELTA;
            } else if (strncmp(prev_bar_pos+1, " RESIDUALS_GAMMA",  minf(substrlen, 16)) == 0) {
                g->residual_coding = BVGRAPH_FLAG_GAMMA;
            } else if (strncmp(prev_bar_pos+1, " RESIDUALS_DELTA",  minf(substrlen, 16)) == 0) {
                g->residual_coding = BVGRAPH_FLAG_DELTA;
            } else if (strncmp(prev_bar_pos+1, " RESIDUALS_ZETA",   minf(substrlen, 15)) == 0) {
                g->residual_coding = BVGRAPH_FLAG_ZETA;
            } else if (strncmp(prev_bar_pos+1, " RESIDUALS_NIBBLE", minf(substrlen, 17)) == 0) {
                g->residual_coding = BVGRAPH_FLAG_NIBBLE;
            } else if (strncmp(prev_bar_pos+1, " REFERENCES_GAMMA", minf(substrlen, 17)) == 0) {
                g->reference_coding = BVGRAPH_FLAG_GAMMA;
            } else if (strncmp(prev_bar_pos+1, " REFERENCES_DELTA", minf(substrlen, 17)) == 0) {
                g->reference_coding = BVGRAPH_FLAG_DELTA;
            } else if (strncmp(prev_bar_pos+1, " REFERENCES_UNARY", minf(substrlen, 17)) == 0) {
                g->reference_coding = BVGRAPH_FLAG_UNARY;
            } else if (strncmp(prev_bar_pos+1, " BLOCK_COUNT_GAMMA",minf(substrlen, 18)) == 0) {
                g->block_count_coding = BVGRAPH_FLAG_GAMMA;
            } else if (strncmp(prev_bar_pos+1, " BLOCK_COUNT_DELTA",minf(substrlen, 18)) == 0) {
                g->block_count_coding = BVGRAPH_FLAG_DELTA;
            } else if (strncmp(prev_bar_pos+1, " BLOCK_COUNT_UNARY",minf(substrlen, 18)) == 0) {
                g->block_count_coding = BVGRAPH_FLAG_UNARY;
            } else if (strncmp(prev_bar_pos+1, " OFFSETS_GAMMA",    minf(substrlen, 14)) == 0) {
                g->offset_coding = BVGRAPH_FLAG_GAMMA;
            } else if (strncmp(prev_bar_pos+1, " OFFSETS_DELTA",    minf(substrlen, 14)) == 0) {
                g->offset_coding = BVGRAPH_FLAG_DELTA;
            } else {
                return bvgraph_property_file_compression_flag_error;
            }
        } else {
            return bvgraph_property_file_compression_flag_error;
        }

        prev_bar_pos = next_bar_pos;
    }

    return 0;
}

/**
 * Parse the properties file for the bvgraph with a given filename.  This 
 * operation will fill in the various fields of the bvgraph structure.
 *
 * This code implements a full parsing of a JAVA property file.
 */
int parse_properties(bvgraph* g)
{
    const int max_property_len = 17;
    const int max_value_len = 1024;

    int rval = 0;

    char *propfilename;
    FILE *propfile;

    // create the filename and open the file
    propfilename = strappend(g->filename,g->filenamelen, ".properties", 11);
    if (!propfilename) { return bvgraph_call_out_of_memory; }

    propfile = fopen(propfilename,"rt");

    // free the memory associated with the filename
    free(propfilename);

    if (!propfile) { return bvgraph_call_io_error; }

    // parse the properties file
    while (!feof(propfile)) 
    {
        int c;

        // this point marks the start of a new natural line
        // that is not a continuation of a logical line 

        // we always ignore any whitespace at the start 
        // of a line
        fskipchars(propfile," \t\f\n\r", 3);

        // stop if we are done
        if (feof(propfile)) { break; }

        // a comment must be the first non-whitespace character
        // check for a comment line
        c = getc(propfile);
        if (c == '#' || c == '!') {
            // this is a comment line and should be ignored
            fnextline(propfile);
            // the file pointer is now at the beginning
            // of the next natural line
        }
        else {    
            char* key;
            char* value;
            size_t key_len;
            uint value_len;

            // return the character
            ungetc(c, propfile);

            // a key 
            key = parse_property_key(propfile, max_property_len);
            if (key == NULL) { return bvgraph_call_out_of_memory; }
            if (key == return_error_string) { return bvgraph_property_file_error; }

            value = parse_property_value(propfile, max_value_len);
            // first free the key
            if (value == NULL || value == return_error_string) { free(key); }
            // then return the exit code
            if (value == NULL) { return bvgraph_call_out_of_memory; }
            if (value == return_error_string) { return bvgraph_property_file_error; }
    
            key_len = strnlen(key, max_property_len);
            value_len = (uint)strnlen(value, max_value_len);

#ifdef MAX_DEBUG
            fprintf(stderr,"\"%s\" = \"%s\"\n", key, value);
#endif 

            //
            // this section of the code interprets the key/value pairs
            //

            if (strncmp(key,"arcs",key_len) == 0) {
                g->m = atoin(value, value_len);
            } else if (strncmp(key,"nodes",key_len) == 0) {
                g->n = atoin(value, value_len);
            } else if (strncmp(key,"maxrefcount",key_len) == 0) {
                g->max_ref_count = atoin(value, value_len);
            } else if (strncmp(key,"windowsize",key_len) == 0) {
                g->window_size = atoin(value, value_len);
            } else if (strncmp(key,"minintervallength",key_len) == 0) {
                g->min_interval_length = atoin(value, value_len);
            } else if (strncmp(key,"zetak",key_len) == 0) {
                g->zeta_k = atoin(value, value_len);
            } else if (strncmp(key,"compressionflags",key_len) == 0) {
                // this function will update the graph structure directly
                rval = parse_compression_flags(g,value,value_len);
            } 
            else if (strncmp(key,"version",key_len) == 0) 
            {
                int version = atoin(value, value_len);
                if (version != 0) { 
                    rval = bvgraph_unsupported_version;
                }
            }     
            else if (strncmp(key,"graphclass",key_len) == 0) 
            {
                if (strncmp(value,"class it.unimi.dsi.webgraph.BVGraph",35)==0) {
                    // this is okay
                } else if (strncmp(value,"it.unimi.dsi.webgraph.BVGraph",29)==0) {
                    // this is okay
                } else {
                    // this is not okay
                    rval = bvgraph_unsupported_version;
                }
            }

            // release the memory
            free(key); free(value);

            // early exit if something went wrong.
            if (rval != 0) { return (rval); }
        }

    }


    return 0;
}


