#include "eflist.h"
#include "bitfile.h"
#include <stdio.h>
#include <stdlib.h>

#include <assert.h>

#include "common.h"

int scan_offsets(const char* filename, size_t *pn, size_t *plargest) {
    int rval = 0;
    bitfile bf = {0};
    uint64_t offset = 0;
    size_t n = 0;
    size_t largest = 0;
    FILE *f;
    
    f = fopen(filename, "rb");
    if (!f) {
        return -1;
    }
    
    rval = bitfile_open(f, &bf);
    if (rval != 0) {
        return rval;
    }
        
    while (!bitfile_eof(&bf)) {
        int g = bitfile_read_gamma(&bf);
        offset += g;
        n++;
        largest = offset;
    }
    
    bitfile_close(&bf);
    fclose(f);
    
    assert(pn != NULL);
    assert(plargest != NULL);
    
    *pn = n;
    *plargest = largest;
    
    return 0;
}

int load_offsets(const char* filename, elias_fano_list *ef, size_t n) {
    int rval = 0;
    bitfile bf = {0};
    uint64_t offset = 0;
    FILE *f;
    
    f = fopen(filename, "rb");
    if (!f) {
        return -1;
    }
    
    rval = bitfile_open(f, &bf);
    if (rval != 0) {
        return rval;
    }
    
    while (n > 0) {
        int g = bitfile_read_gamma(&bf);
        offset += g;
        eflist_add(ef, g);
        n--;
    }
    
    bitfile_close(&bf);
    fclose(f);
    
    return 0;
}

int test_offsets(elias_fano_list *ef, size_t n, uint64_t ntrials) {
    int64_t result = 0, rval = 0;;
    while (ntrials-- > 0) {
        uint64_t index = xorshift64star() % n;
        result = eflist_get(ef, (int64_t)index);
        rval ^= (result & 0x01); 
    }
    return (int)rval;    
}

int main_func(const char* filename, uint64_t ntrials) {
    int rval;
    elias_fano_list eflist;    
    size_t n, largest; 
    double t0, t1;
    
    printf("STATUS  Scanning offsets file %s\n", filename);
    t0 = get_wall_time();
    if (scan_offsets(filename, &n, &largest) != 0) {
        fprintf(stderr, "error scanning offsets, most likely file does not exist\n");
        return 1;
    }
    t1 = get_wall_time();    
    printf("STATUS  Finished scanning offsets.\n");
    printf("TIME    %.2f sec\n", t1-t0);
    
    printf("RESULT  n=%" PRINTF_INT64_MODIFIER "d\n", (int64_t)n);
    printf("RESULT  largest=%" PRINTF_INT64_MODIFIER "d\n", (int64_t)largest);

    printf("STATUS  Loading offsets.\n");    
    t0 = get_wall_time();
    rval = eflist_create(&eflist, n, largest);
    if (rval != 0) {
        fprintf(stderr, "problem creating eflist, rval=%i\n", rval);
    }
    rval = load_offsets(filename, &eflist, n);
    if (rval != 0) {
        fprintf(stderr, "problem filling eflist, rval=%i\n", rval);
    }
    t1 = get_wall_time();
    printf("STATUS  Finished loading offsets.\n");
    printf("TIME    %.2f sec\n", t1-t0);
    
    printf("STATUS  Testing offsets.\n");
    t0 = get_wall_time();
    test_offsets(&eflist, n, ntrials);
    t1 = get_wall_time();
    printf("STATUS  Finished testing offsets.\n");     
    printf("TIME    %.2f sec\n", t1-t0);
    printf("RATE    %.1f offsets/sec\n", (double)(ntrials)/(t1-t0));
    
    
    eflist_free(&eflist);
        
    
    return 0;
    
}

#ifdef _WIN32
long long atoll(const char* str) {
    return (long long)(_atoi64(str));
}
#endif

int main(int argc, const char** argv) {

    if (argc != 3) {
        fprintf(stderr, "usage: eflist_perf offsetfile ntrials\n");
        return -1;
    }
    
    return main_func(argv[1], atoll(argv[2]));
    
}