/*
 * David Gleich
 * Copyright, Stanford University, 2007
 * 7 June 2007
 */

/**
 * @file bvpagerank.cc
 * Implement a few simple PageRank algorithms as tests of the library.
 */

extern "C" {
#include "bvgraph.h"
}

#include <vector>
#include <iostream>
#include <sstream>

#include <math.h>

void power_alg(bvgraph* g, double alpha, double tol, int maxiter, std::vector<double>& prvec);
void inner_outer_alg(bvgraph* g, double alpha, double tol, int maxiter, std::vector<double>& prvec);
void updated_richardson_alg(bvgraph *g, double alpha, double tol, int maxit, std::vector<double>& prvec);

void print_usage(std::ostream& cerr)
{
    using namespace std;
    cerr << "usage: bvpagerank graphfile output alg alpha tol maxit" << endl;
    cerr << endl;
    cerr << "Solves the PageRank problem on a boldi-vigna graph" << endl;
    cerr << "with uniform teleportation in the strongly preferential model." << endl;
    cerr << endl;
    cerr << "For any option except graphfile, specify _ to get the default value." << endl;
    cerr << endl;
    cerr << "graphfile: the base filename of a bvgraph file.  (e.g. cnr-2000)" << endl;
    cerr << "output: output filename for pagerank vector, \'none\' means no output" << endl;
    cerr << "alg: the PageRank computation algorithm [\'power\', \'inout\' or \'rich\']" << endl;
    cerr << "alpha: the value of alpha in the PageRank [float from 0 to 1]" << endl;
    cerr << "tol: the stopping tolerance for the 1-norm error [float from (0,Infinity])" << endl;
    cerr << "maxit: the maximum number of iterations [positive integer]" << endl;
    cerr << endl;
    cerr << "defaults: " << endl;
    cerr << "  output = \'none\'; alg = \'inout\'; alpha = 0.85; tol = 1e-8; maxit = 10000" << endl;
    cerr << endl;
}

int main(int argc, char **argv)
{
    using namespace std;

    if (argc < 2 || argc > 7) {
        print_usage(std::cerr);
        return (-1);
    }

    std::string graphfilename;
    
    enum alg_tag {
        bvpagerank_alg_power,
        bvpagerank_alg_inout,
        bvpagerank_alg_rich
    };
    enum alg_tag alg = bvpagerank_alg_inout;

    bool output = false;
    std::string outputfilename;
    double alpha = 0.85;
    double tol = 1e-8;
    int maxit = 10000;
    
    graphfilename = argv[1];

    if (argc > 2) {
        std::string outputarg = argv[2];
        if (outputarg.compare("none") == 0 || outputarg.compare("_") == 0) {
            // don't change the default
        }
        else {
            output = true;
            outputfilename = outputarg;
        }
    }

    if (argc > 3) {
        std::string algarg = argv[3];
        if (algarg.compare("_") == 0) {
            // don't change the default
        }
        else if (algarg.compare("power") == 0) {
            alg = bvpagerank_alg_power;
        } 
        else if (algarg.compare("inout") == 0) {
            alg = bvpagerank_alg_inout;
        }
        else if (algarg.compare("rich") == 0) {
            alg = bvpagerank_alg_rich;
        }
    }

    if (argc > 4) {
        std::string alphaarg = argv[4];
        if (alphaarg.compare("_") != 0) {
            std::stringstream ss(alphaarg);
            ss >> alpha;
        }
    }

    if (argc > 5) {
        std::string tolarg = argv[4];
        if (tolarg.compare("_") != 0) {
            std::stringstream ss(tolarg);
            ss >> tol;
        }
    }

    cout << "Parameters: " << endl;
    cout << "   graphfile = " << graphfilename << endl;
    if (output) { cout << "  outputfile = " << outputfilename << endl; }
    else { cout << "      output = none" << endl; }
    cout << "         alg = "; 
    switch (alg) {
        case bvpagerank_alg_power: cout << "power"; break;
        case bvpagerank_alg_inout: cout << "inout"; break;
        case bvpagerank_alg_rich: cout << "rich"; break;
    }
    cout << endl;
    cout << "       alpha = " << alpha << endl;
    cout << "         tol = " << tol << endl;
    cout << endl;

    bvgraph g = {0};
    int rval;

    rval = bvgraph_load(&g, graphfilename.c_str(), (unsigned int)graphfilename.length(), 0);
    if (rval) {
        cerr << "error: " << bvgraph_error_string(rval) << endl;
        return (rval);
    }

    cout << "nodes = " << g.n << endl;
    cout << "edges = " << g.m << endl;

    // initialize the PageRank vector with a value of 1/n;
    std::vector<double> prvec(g.n,1.0/(double)g.n);

    if (alg == bvpagerank_alg_power) {
        power_alg(&g, alpha, tol, maxit, prvec);
    }
    else if (alg == bvpagerank_alg_inout) {
        inner_outer_alg(&g, alpha, tol, maxit, prvec);
    }
    else if (alg == bvpagerank_alg_rich) {
        updated_richardson_alg(&g, alpha, tol, maxit, prvec);
    }
    else {
        cerr << "error: unknown algorithm" << endl;
        return (-1);
    }

    bvgraph_close(&g);

    if (output) {
        // 
    }

    return (0);
}


double sum(double *x, size_t n)
{
    double rval = 0.0;
    while (n-- > 0) {
        rval += *x; x++;
    }
    return (rval);
}

void shift(double *x, double s, size_t n)
{
    while (n-- > 0) {
        *x += s; x++;
    }
}

void set(double *x, double s, size_t n)
{
    while (n-- > 0) {
        *x = s; x++;
    }
}

double diff_norm_1(double* x, double *y, size_t n)
{
    double rval = 0.0;
    while (n-- > 0) {
        rval += fabs(*x - *y);
        x++; y++;
    }
    return (rval);
}

int mult(bvgraph *g, double *x, double *y, double alpha)
{
    using namespace std;
    bvgraph_iterator git;
    int rval = bvgraph_nonzero_iterator(g, &git);
    if (rval) {
        cerr << "error: cannot get bvgraph iterator " << endl;
        cerr << "bvgraph error: " << bvgraph_error_string(rval) << endl;
        cerr << "halting iteration..." << endl;
        return (-1);
    }

    int64_t *links; uint64_t i, d;
    double id=0.0;

    for (; 
         bvgraph_iterator_valid(&git); 
         bvgraph_iterator_next(&git))
    {
        bvgraph_iterator_outedges(&git, &links, &d);
        if (d > 0) { id = 1.0/(double)d; }
        for (i = 0; i < d; i++) {
            y[links[i]] += alpha*x[git.curr]*id;
        }
    }
    bvgraph_iterator_free(&git);
    return (0);
}

void power_alg(bvgraph *g, double alpha, double tol, int maxit, std::vector<double>& prvec)
{
    using namespace std;

    size_t n = (size_t)g->n;

    std::vector<double> vec2(n);
    double *x = &prvec[0];
    double *y = &vec2[0];

    int iter = 0;
    while (iter < maxit) {
        if (mult(g, x, y, alpha)) { return; }

        double w = 1.0 - sum(y, n);
        shift(y, w*(1.0/(double)n),n);
        double delta = diff_norm_1(x, y, (size_t)n);

        cout << iter << "  " << delta << std::endl;

        if (delta < tol) {
            break;
        }

        {
            double *temp;
            temp = x;
            x = y;
            y = temp;
        }

        // reset y
        set(y, 0.0, n);

        iter++;
    }
}

int dangling_mult(bvgraph *g, double *x, double *y, size_t n)
{
    using namespace std;
    bvgraph_iterator git;
    int rval = bvgraph_nonzero_iterator(g, &git);
    if (rval) {
        cerr << "error: cannot get bvgraph iterator" << endl;
        cerr << "bvgraph error: " << bvgraph_error_string(rval) << endl;
        cerr << "halting iteration..." << endl;
        return (-1);
    }

    int64_t *links; uint64_t i, d;
    double id=0.0;

    for (; 
         bvgraph_iterator_valid(&git); 
         bvgraph_iterator_next(&git))
    {
        bvgraph_iterator_outedges(&git, &links, &d);
        if (d > 0) { id = 1.0/(double)d; }
        for (i = 0; i < d; i++) {
            y[links[i]] += x[git.curr]*id;
        }
    }
    bvgraph_iterator_free(&git);
    double w = 1.0 - sum(y,g->n);
    shift(y, w*(1.0/(double)g->n), g->n);
    return (0);
}

double compute_outer_residual(double *x, double *y, double alpha, size_t n)
{
    double rval = 0.0; double in = 1./(double)n;
    while (n-->0) { rval += fabs(alpha*(*y++) - *x++ + (1-alpha)*in); }
    return (rval);
}

double compute_inner_residual(double *x, double *y, double *f, double beta, size_t n)
{
    double rval = 0.0; 
    while (n-->0) { rval += fabs(*f++ + beta*(*y++)-*x++); }
    return (rval);
}

void compute_f(double *f, double *y, double alpha, double beta, size_t n)
{
    double amb = alpha-beta; double vi=(1.0 -alpha)/(double)n;
    while (n-->0) { *f++ = amb*(*y++) + vi; }
}

void axpysz(double *z, double *x, double *y, double alpha, size_t n) 
{
    while (n-->0) { *z++ = alpha*(*x++) + (*y++); }
}

void compute_power_start(double *x, double *y, double alpha, size_t n)
{
    double vi=(1.0 -alpha)/(double)n;
    while (n-->0) { *x = alpha*(*y++) + vi; }
}

void inner_outer_alg(bvgraph *g, double alpha, double tol, int maxit, std::vector<double>& prvec)
{
    using namespace std;
    double beta = 0.5;
    double itol = 0.2;

    size_t n = (size_t)g->n;

    std::vector<double> vec2(n); 
    std::vector<double> vec3(n);

    double *x = &prvec[0];
    double *y = &vec2[0];
    double *f = &vec3[0];

    if (dangling_mult(g, x, y, n)) { return; }
    double odelta = compute_outer_residual(x, y, alpha, n);
    int iter = 1;
    while (odelta > tol && iter < maxit) {
        int iiter=0; compute_f(f, y, alpha, beta, n);
        while (iter+iiter < maxit && compute_inner_residual(x,y,f,beta,n) > itol) {
            axpysz(x,y,f,beta,n);
            set(y, 0.0, n);
            dangling_mult(g, x, y, n); iiter++;
        }
        iter+=iiter; if (iiter == 0) { compute_power_start(x,y,alpha,n); break; }
        odelta = compute_outer_residual(x,y,alpha,n);
        cout << "Outer: " << iter << " " << odelta << endl;
    }
    while (odelta > tol && iter++ < maxit) {
        set(y, 0.0, n);
        if (mult(g, x, y, alpha)) { return; }
        double w = 1.0 - sum(y, n);
        shift(y, w*(1.0/(double)n),n);
        odelta = diff_norm_1(x, y, (size_t)n);

        { double *temp; temp = x; x = y; y = temp; }

        // reset y
        set(y, 0.0, n);

        cout << "Power: " << iter << " " << odelta << endl;
    }
}

/**
 * From the computat alpha*P'*x, compute the initial
 * residual for the updated richardson iteration so that 
 * aPx on exit is v - (I - aP')x
 */
void initial_residual(double *aPx, double *x, size_t n) {
    double in = (1./(double)n);
    while (n-- > 0) { *aPx += in - *x++; }
}

double compute_pagerank_residual(double *y, double alpha, double nx, double dtx, size_t n)
{
    double rval=0.0; double in = (1./(double)n); double inx=1.0/nx;
    double avi = (1-alpha+alpha*dtx*inx)*in;
    while (n-- > 0) { rval += fabs((*y++-in)*inx + avi); }
    return (rval);
}

int updated_richardson_iter(bvgraph *g, double *x, double *y, double alpha, size_t n, double *pnx, double *pdtx)
{
    double nx=0.0, dtx=0.0;
    using namespace std;
    bvgraph_iterator git;
    int rval = bvgraph_nonzero_iterator(g, &git);
    if (rval) {
        cerr << "error: cannot get bvgraph iterator" << endl;
        cerr << "bvgraph error: " << bvgraph_error_string(rval) << endl;
        cerr << "halting iteration..." << endl;
        return (-1);
    }

    int64_t *links; uint64_t i, d;
    double id=0.0;

    for (; 
         bvgraph_iterator_valid(&git); 
         bvgraph_iterator_next(&git))
    {
        bvgraph_iterator_outedges(&git, &links, &d);
        double yj = y[git.curr]; x[git.curr] += yj; nx += x[git.curr];
        if (d > 0) { id = 1.0/(double)d; }
        else { dtx += x[git.curr]; }
        for (i = 0; i < d; i++) {
            y[links[i]] += yj*alpha*id;
        }
        y[git.curr] -= yj;
    }
    bvgraph_iterator_free(&git);
    if (pnx) { *pnx = nx; }
    if (pdtx) { *pdtx = dtx; }
    return (0);
}

void updated_richardson_alg(bvgraph *g, double alpha, double tol, int maxit, std::vector<double>& prvec)
{
    using namespace std;

    size_t n = (size_t)g->n;

    std::vector<double> vec2(n);
    double *x = &prvec[0];
    double *y = &vec2[0];
    
    double nx=1.0,dtx=0.0; // norm(x,1) and d'*x
    if (mult(g, x, y, alpha)) { return; }
    initial_residual(y, x, n);
    double delta = 1.0; int iter=1;
    while (delta > tol && iter++ < maxit) {
        updated_richardson_iter(g,x,y,alpha,n,&nx,&dtx);
        delta = compute_pagerank_residual(y,alpha,nx,dtx,n);
        cout << iter << " " << delta << endl;
    }
}

