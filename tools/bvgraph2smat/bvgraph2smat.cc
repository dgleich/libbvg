/*
 * David F. Gleich
 * Copyright, Purdue University, 2011
 */
 
/** @file bvgraph2smat.cc
 * Output a graph in smat or bsmat format.
 */

#include <iostream>
#include <string>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bvgraph.h"

using namespace std;

bool write_smat_format(bvgraph *g, bvgraph_iterator *git, const char *outname) {
    FILE *f = fopen(outname,"wt");
    if (!f) {
        fprintf(stderr, "error: cannot open %s for writing", outname);
        return false;
    }
    
    fprintf(f, "%lld %lld %lld\n", g->n, g->n, g->m);
    
    int64_t *links; uint64_t i, d;
    for (; 
         bvgraph_iterator_valid(git); 
         bvgraph_iterator_next(git))
    {
        bvgraph_iterator_outedges(git, &links, &d);
        for (i = 0; i < d; i++) {
            fprintf(f, "%lld %lld 1\n", git->curr, links[i]);
        }
    }
    fclose(f);
    return true;
}

bool write_bsmat_format(bvgraph *g, bvgraph_iterator *git, const char *outname) {
    FILE *f = fopen(outname,"w");
    if (!f) {
        fprintf(stderr, "error: cannot open %s for writing", outname);
        return false;
    }
    
    fwrite(&g->n, sizeof(int), 1, f);
    fwrite(&g->n, sizeof(int), 1, f);
    fwrite(&g->m, sizeof(int), 1, f);
    
    double val=1.0;
    int64_t *links; uint64_t i, d;
    for (; 
         bvgraph_iterator_valid(git); 
         bvgraph_iterator_next(git))
    {
        bvgraph_iterator_outedges(git, &links, &d);
        for (i = 0; i < d; i++) {
            fwrite(&git->curr, sizeof(int64_t), 1, f);
            fwrite(&links[i], sizeof(int64_t), 1, f);
            fwrite(&val, sizeof(double), 1, f);
        }
    }
    fclose(f);
    return true;
}

bool write_badj_format(bvgraph *g, bvgraph_iterator *git, const char *outname) {
    FILE *f = fopen(outname,"w");
    if (!f) {
        fprintf(stderr, "error: cannot open %s for writing", outname);
        return false;
    }
    
    int64_t n = (int64_t) g->n;
    int64_t m = (int64_t) g->m;

    fwrite(&n, sizeof(int64_t), 1, f);
    fwrite(&m, sizeof(int64_t), 1, f);
    
    if (n > 4294967294)
    {
        int64_t *links; uint64_t i, d;
        for (; 
            bvgraph_iterator_valid(git); 
            bvgraph_iterator_next(git))
        {
            bvgraph_iterator_outedges(git, &links, &d);
            fwrite(&d, sizeof(int64_t), 1, f);
            for (i = 0; i < d; i++) {
                fwrite(&links[i], sizeof(int64_t), 1, f);
            }
        }
    }
    else
    {
        int64_t *links; uint64_t i, d;
        uint32_t val;
        for (;
             bvgraph_iterator_valid(git);
             bvgraph_iterator_next(git))
        {
            bvgraph_iterator_outedges(git, &links, &d);
            val = (uint32_t) d;
            fwrite(&val, sizeof(uint32_t), 1, f);
            for (i = 0; i < d; i++)
            {
                val = (uint32_t) links[i];
                fwrite(&val, sizeof(uint32_t), 1, f);
            }
        }
    }
    
    fclose(f);
    return true;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        cout << "usage: bvgraph2smat bvgraph [-f bsmat|smat|badj] [-o outputname]" 
             << endl;
    }
    
    const char* bvgraphname = argv[1];
    
    const char* formatstr = "smat";
    enum { SMAT_FORMAT=1, BSMAT_FORMAT, BADJ_FORMAT } format=SMAT_FORMAT;
    
    string outputname;
    
    for (int argi=2; argi < argc; ++argi) {
        if (argi + 1 >= argc) {
            cout << "unknown singleton argument \'" 
                 << argv[argi] << "\'" << endl;
        } else {
            if (strcmp(argv[argi],"-f")==0) {
                if (strcmp(argv[argi+1],"smat") == 0) {
                    formatstr = "smat";
                    format = SMAT_FORMAT;
                } else if (strcmp(argv[argi+1],"bsmat") == 0) {
                    formatstr = "bsmat";
                    format = BSMAT_FORMAT;
                } else if (strcmp(argv[argi+1],"badj") == 0) {
                    formatstr = "badj";
                    format = BADJ_FORMAT;
                } else {
                    cout << "unknown format \'" 
                         << argv[argi+1] << "\'" << endl;
                    return (-1);
                }
                argi++;
            } else if (strcmp(argv[argi], "-o") == 0) {
                outputname = argv[argi+1];
                argi++;
            } else {
                cout << "unknown argument \'" 
                     << argv[argi] << "\'" << endl;
                return (-1);
            }
        }
    }
    
    if (outputname.empty()) {
        outputname = string(bvgraphname) + "." + formatstr;
    }
    
    cout << "Reading bvgraph " << bvgraphname << endl;
    cout << "Writing " << formatstr << " file " << outputname << endl;
    
    bvgraph g = {};
    int rval;

    rval = bvgraph_load(&g, bvgraphname, strlen(bvgraphname), -1);
    if (rval) {
        cerr << "error: " << bvgraph_error_string(rval) << endl;
        return (rval);
    }
    
    bvgraph_iterator git;
    rval = bvgraph_nonzero_iterator(&g, &git);
    if (rval) {
        cerr << "error: cannot get bvgraph iterator " << endl;
        cerr << "bvgraph error: " << bvgraph_error_string(rval) << endl;
        cerr << "halting iteration..." << endl;
        return (-1);
    }
    
    switch (format) {
        case SMAT_FORMAT:
            write_smat_format(&g, &git, outputname.c_str());
            break;
        case BSMAT_FORMAT:
            write_bsmat_format(&g, &git, outputname.c_str());
            break;
        case BADJ_FORMAT:
            write_badj_format(&g, &git, outputname.c_str());
            break;
    }
    
    bvgraph_iterator_free(&git);
    bvgraph_close(&g);
        
    
}
 
