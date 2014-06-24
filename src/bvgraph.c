/*
 * David Gleich
 * Copyright, Stanford University, 2007
 * 17 May 2007
 */

/**
 * @file bvgraph.c
 * Implement all the routines to work with bvgraph files in the bvg library.
 */
 
/** History
 *  01-24-2008: Added function to initialize structure memory to fix a 
 *              segfault on the 32-bit version.
 */

#include "bvgraph_internal.h"
#include "bvgraph_inline_io.h"

/**
 * Define all the error codes
 */

const int bvgraph_call_out_of_memory = -1;
const int bvgraph_call_io_error = -2;
const int bvgraph_call_unsupported = -3;
const int bvgraph_load_error_filename_too_long = 11;
const int bvgraph_load_error_buffer_too_small = 12;
const int bvgraph_property_file_error = 21;
const int bvgraph_unsupported_version = 22;
const int bvgraph_property_file_compression_flag_error = 23;
const int bvgraph_vertex_out_of_range = 31;
const int bvgraph_requires_offsets = 32;
const int bvgraph_unsupported_coding = 33;

/**
 * This function sets the default options in a graph
 */

static void set_defaults(bvgraph *g)
{
    g->zeta_k = 3;
    g->window_size = 7;
    g->min_interval_length = 3;
    g->max_ref_count = 3;
}

bvgraph *bvgraph_new(void)
{
    bvgraph *g;
    g = malloc(sizeof(bvgraph));

    return (g);
}

void bvgraph_free(bvgraph *g)
{
    free(g);
    g = NULL;
}

/**
 * Load the metadata associated with a bvgraph.  Largely, this involves
 * just parsing the properties files.
 *
 * @param g a newly created bvgraph structure
 * @param filename the base filename for a set of bvgraph files, 
 * so filename.graph and filename.properties must exist.
 * @param offset_step controls how many offsets are loaded, 
 * if offset_step = -1, then the graph file isn't loaded into memory
 * if offset_step = 0, then the graph file is loaded, but no offsets
 * no other values are supported at the moment.
 * @return 0 if successful
 * bvgraph_load_error_filename_too_long - indicates the filename was too long
 *
 * @example
 * bvgraph g; bvgraph_load(&b, "cnr-2000", 0);
 */
int bvgraph_load(bvgraph* g, const char *filename, unsigned int filenamelen, int offset_step)
{
    // this call will treat all the memory as internal
    return bvgraph_load_external(g, filename, filenamelen, offset_step, NULL, 0, NULL, 0);
}  

/**
 * Load a graph file but using a set of externally provided buffers 
 * for the data.  This might be useful in the case that you want to managed
 * memory for the graph independently of this function.  
 *
 * To maintain the graph, only the bvgraph structure, and the two
 * external ararys: gmemory and offsets are required.  Consequently,
 * a bvgraph structure can always be patched together out of these
 * functions.
 *
 * One common way of using this function would be to first
 * load a graph on the disk (bvgraph_load(...,offset_step=-1))
 * and then call bvgraph_required_memory with an alternative
 * offset step.
 *
 * @param g the bvgraph structure
 * @param g a newly created bvgraph structure
 * @param filename the base filename for a set of bvgraph files, 
 * so filename.graph and filename.properties must exist.
 * @param offset_step controls how many offsets are loaded, 
 * if offset_step = -1, then the graph file isn't loaded into memory
 * if offset_step = 0, then the graph file is loaded, but no offsets
 * no other values are supported at the moment.
 * @param gmemory an arry of size gmemsize for the graph 
 * (if NULL, then this parameter is treated as internal memory)
 * @param gmemsize the size of the gmemory block
 * @param offsets an array of offsets
 * (if NULL, then this parameter is treated as internal memory)
 * @param offsetssize the number of offsets
 * @return 0 if successful
 * bvgraph_load_error_filename_too_long - indicates the filename was too long
 */
int bvgraph_load_external(bvgraph *g,
                          const char *filename, unsigned int filenamelen, int offset_step,
                          unsigned char *gmemory, size_t gmemsize,
                          unsigned long long* offsets, int offsetssize)
{
    int rval = 0;

    assert(offset_step == 0 || offset_step == -1 || offset_step == 1);

    if (filenamelen > BVGRAPH_MAX_FILENAME_SIZE-1) { 
        return bvgraph_load_error_filename_too_long;
    }
    
    memset(g,0,sizeof(bvgraph));

    strncpy(g->filename, filename, filenamelen);
    g->filename[filenamelen] = '\0';
    g->filenamelen = filenamelen;

    g->offset_step = offset_step;

    set_defaults(g);

    rval = parse_properties(g);
    // check for any errors
    if (rval != 0) { return rval; }

    // continue processing
    if (offset_step >= 0) 
    {
        if (offset_step == 0 || offset_step == 1) {    //modified 082911
            // in this case, we ust load the graph
            // file into memory

            // first get the filesize
            unsigned long long graphfilesize;
            char *gfilename = strappend(g->filename, g->filenamelen, ".graph", 6);
            rval = fsize(gfilename, &graphfilesize);
            free(gfilename);
            if (rval) { 
                return bvgraph_call_io_error;
            }

            if (gmemory != NULL) {
                // they want to use their own memory, make sure
                // they allocated enough!
                if (gmemsize < graphfilesize) {
                    return bvgraph_load_error_buffer_too_small;
                }
                g->memory = gmemory;
                g->memory_size = gmemsize;
                g->memory_external = 1;
            } else {
                // we have to allocate the memory ourselves
                g->memory_size = (size_t)graphfilesize;
                g->memory = malloc(sizeof(unsigned char)*g->memory_size);
                if (!g->memory) {
                    return bvgraph_call_out_of_memory;
                }
                g->memory_external = 0;
            }

            // now read the file
            gfilename = strappend(g->filename, g->filenamelen, ".graph", 6);
            {
                size_t bytesread = 0;
                FILE *gfile = fopen(gfilename, "rb");
                free(gfilename);
                if (!gfile) {
                    return bvgraph_call_io_error;
                }
                bytesread = fread(g->memory, 1, g->memory_size, gfile);
                if (bytesread != graphfilesize) {
                    return bvgraph_call_io_error;
                }
                fclose(gfile);
            }
            // we now have the graph in memory!

            if (offset_step == 1) {        //modified 082911
                if (offsets != NULL) {
                    g->offsets = offsets;
                    g->offsets_external = 1;
                } else {
                    // we have to allocate the memory ourselves
                    g->offsets = (unsigned long long*) malloc(sizeof(unsigned long long)*g->n);
                    g->offsets_external = 0;
                }
                // now read the file
                char *ofilename = strappend(g->filename, g->filenamelen, ".offsets", 8);
                bitfile bf;
                long long off = 0;
                long long i;
                g->offsets = (unsigned long long*)malloc(g->n*sizeof(unsigned long long));
                FILE *ofile = fopen(ofilename, "rb");
                if (ofile) {
                    rval = bitfile_open(ofile, &bf);
                    if (rval) {
                        return bvgraph_call_io_error;
                    }
                    for (i = 0; i < g->n; i++){
                        off = read_offset(g, &bf) + off;
                        g->offsets[i] = off;
                    }
                } else {
                    // need to build the offsets
                    bvgraph_iterator git;
                    int rval = bvgraph_nonzero_iterator(g, &git);
                    if (rval) { return rval; }
                    g->offsets[0] = 0;
                    for (; bvgraph_iterator_valid(&git); bvgraph_iterator_next(&git)) {
                        if (git.curr+1 < g->n) {
                            g->offsets[git.curr+1] = bitfile_tell(&git.bf);
                        }
                    }
                    bvgraph_iterator_free(&git);
                }
            }
        }
    }
    else
    {
        g->memory_size = 0;
    }
     
    // presently the othercases are not supported, so we don't have to
    // worry about them!

    return (0);
}

/**
 * Close a bvgraph object.  This operation will free any associated memory.
 * However, it will leave any existing iterators in an zombie state.  These
 * iterators should be closed before closing the graph, but this is only
 * recommended to avoid bugs.
 *
 * TODO Implement a "valid" function that all the iterators would check?
 *
 * @param g the graph
 * @return 0 on success
 */
int bvgraph_close(bvgraph* g)
{
    if (!g->memory_external) { free(g->memory); }
    if (!g->offsets_external) { free(g->offsets); }
    memset(g, 0, sizeof(bvgraph));

    return (0);
}

/**
 * Compute the memory required to load a bvgraph into memory
 * with the desired offset_step.  
 *
 * The best way to use this function is to load the bvgraph
 * with offset_step = -1, and then call this function to determine
 * how much additional memory is required.  
 *
 * @param g the graph
 * @param offset_step the new offset_step value
 * @param gbuf the size of the graph buffer
 * @param offsetbuf the size of the offset buffer
 * @return 0 on success
 */
int bvgraph_required_memory(bvgraph *g, int offset_step, size_t *gbuf, size_t *offsetbuf)
{
    if (offset_step < 0) {
        if (gbuf) { *gbuf = 0; }
        if (offsetbuf) { *offsetbuf = 0; }
    }
    else if (offset_step < 2) {
        unsigned long long graphfilesize;
        char *gfilename = strappend(g->filename, g->filenamelen, ".graph", 6);
        int rval = fsize(gfilename, &graphfilesize);
        free(gfilename);
        if (rval) { 
            return bvgraph_call_io_error;
        }

        if (gbuf) { *gbuf = (size_t)graphfilesize; }
        // always set the offsetbuf here even if we are about to change
        // it.
        if (offsetbuf) { *offsetbuf = 0; }

        if (offset_step == 1) {
            if (offsetbuf) { *offsetbuf = sizeof(unsigned long long)*g->n; }
        }
    }
    else {
        return bvgraph_call_unsupported;
    }

    return (0);
}

/** Access the outdegree for a given node.
 *
 * For a random access graph, this method provides a thread-safe means of 
 * getting the outdegree for a given node.  If you are going to make
 * many sequential outdegree calls from a single thread, the 
 * random_access_iterator structure is much more efficient.
 *
 * To use this method, the graph must be loaded with offsets.
 *
 * @param g the bvgraph structure with offsets loaded
 * @param x the node
 * @param[out] d the outdegree
 * @return 0 on success
 */

int bvgraph_outdegree(bvgraph *g, long long x, unsigned long long *d) 
{
    bvgraph_random_iterator ri;
    int rval = bvgraph_random_access_iterator(g, &ri);
    if (rval == 0) {
        rval = bvgraph_random_outdegree(&ri, x, d);
    }
    bvgraph_random_free(&ri);
    return (rval);
}

/** Access the successors for a given node.
 *
 * For a random access graph, this method provides a thread-safe means of 
 * getting the outdegree for a given node.  If you are going to make
 * many sequential outdegree calls from a single thread, the 
 * random_access_iterator structure is much more efficient.
 *
 * To use this method, the graph must be loaded with offsets.
 *
 * @param g the bvgraph structure with offsets loaded
 * @param x the node
 * @param[out] start the starting point of successor list
 * @param[out] length the length of successor list (degree)
 * @return 0 on success
 */

int bvgraph_successors(bvgraph *g, long long x, long long** start, unsigned long long *length) 
{
    bvgraph_random_iterator ri;
    int rval = bvgraph_random_access_iterator(g, &ri);
    if (rval == 0) {
        bvgraph_random_successors(&ri, x, start, length);
        //bvgraph_random_free(&ri);
        return (0);
    } else { 
        return (rval); 
    }
}


/**
 * Return an error string associated with an error code.
 * 
 * @param code the error code
 * @return the pointer to a string associated with an error code.
 */
const char* bvgraph_error_string(int code)
{
    if (code == bvgraph_call_out_of_memory) {
        return "malloc error --- probably out of memory";
    }
    else if (code == bvgraph_call_io_error) {
        return "io error --- probably file not found";
    }
    else if (code == bvgraph_call_unsupported) {
        return "the call tried to perform an unsupported operation";
    }
    else if (code == bvgraph_load_error_filename_too_long) {
        return "filename too long to store";
    }
    else if (code == bvgraph_load_error_buffer_too_small) {
        return "one of the provided buffers was too small";
    }
    else if (code == bvgraph_property_file_error) {
        return "the property file is not a valid property file format";
    }
    else if (code == bvgraph_property_file_compression_flag_error) {
        return "the property file contained an unknown compression flag";
    }
    else if (code == bvgraph_call_io_error) {
        return "the file version is not supported";
    }
    else if (code == bvgraph_vertex_out_of_range){
        return "vertex is out of range";
    }
    else if (code == bvgraph_requires_offsets){
        return "offsets are required";
    }
    else if (code == bvgraph_unsupported_coding){
        return "coding unsupported";
    }
    else if (code == 0) {
        return "the call succeeded";
    }
    else {
        return "unknown error";
    }
}
