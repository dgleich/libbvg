#include <stdio.h>
#include <stdlib.h>
#include "../include/bvgraph.h"
#include "../include/bvgraphfun.h"
#include "../include/bitfile.h"
#include <string.h>
#include <time.h>

void run_iteration(bvgraph g){
	bvgraph_iterator git;
	bvgraph_nonzero_iterator(&g, &git);

	for (; bvgraph_iterator_valid(&git); bvgraph_iterator_next(&git)) {
		int *links; unsigned int d;
		bvgraph_iterator_outedges(&git, &links, &d);
		printf("node %i has degree %d\n", git.curr, d);
		int i = 0;
		for (i; i<d; ++i) {
			printf("node %i links to node %i\n", git.curr, links[i]);
		}
	}
	bvgraph_iterator_free(&git);
}

void head_tail_first_test(bvgraph g){
	//test random access from 0, n-1, 1, n-2, 2, n-3, ...
	int i, j;
	unsigned int d;

	for (i = 0; i <= g.n; i++){
		int node;
		if ( i % 2 == 0){
			node = i / 2;
		}
		else {
			node = g.n - i / 2;
		}
		//get successors example
		int *links;	
		bvgraph_successors(&g, node, &links, &d);
		printf ("node %d has degree %d\n", node, d);

		int j = 0;
		for (j; j< d; j++){
			printf("node %i links to node %i\n", node, links[j]);
		}
	}

}

void random_test(bvgraph g, int test_num){

	//randomly generate test case
	int i = 0;
	unsigned int d;
	srand(time(NULL));

	for (i; i < test_num; i++){
		int node = rand() % g.n;
		int *links;

		bvgraph_successors(&g, node, &links, &d);
		printf ("node %d has degree %d\n", node, d);

		int j = 0;
		for (j; j< d; j++){
			printf("node %i links to node %i\n", node, links[j]);
		}
	}
	printf("Total %d random nodes generated and tested.\n", test_num);

}

void test_iteratively(bvgraph g){
	int i, rval;
	unsigned int d;

	for (i = 0; i < g.n; i++){
		//get successors example
		int *links;
		bvgraph_successors(&g, i, &links, &d);
		printf ("node %d has degree %d\n", i, d);
		
		int j = 0;
		for (j; j< d; j++){
			printf("node %i links to node %i\n", i, links[j]);
		}
	}
}

int main(int argc, char** argv){
	bvgraph g = {0};
	//const char* name = "libbvg/data/wb-cs.stanford";
	const char* name = argv[1];
	const char* method = argv[2];

	//load with offsets
	bvgraph_load(&g, name, strlen(name), 1);

	printf("nodes = %d\n", g.n);
	printf("edges = %lli\n", g.m);

	if (strcmp(method, "iterate") == 0){
		test_iteratively(g);
	}
	else if (strcmp(method, "random") == 0){
		int num = atoi(argv[3]);
		random_test(g, num);
	}
	else if (strcmp(method, "head-tail") == 0){
		head_tail_first_test(g);
	}
	else{
		printf("Wrong parameters. Stop.\n");
		return 1;
	}
	
	//random access test
	//test all nodes iteratively by random access on each iteration
	//iteration demo
	/*
	bvgraph_iterator git;
	bvgraph_nonzero_iterator(&g, &git);

	for (; bvgraph_iterator_valid(&git); bvgraph_iterator_next(&git)) {
		int *links; unsigned int d;
		bvgraph_iterator_outedges(&git, &links, &d);
		printf("node %i has degree %d\n", git.curr, d);
		int i = 0;
		for (i; i<d; ++i) {
			printf("node %i links to node %i\n", git.curr, links[i]);
		}
	}
	bvgraph_iterator_free(&git);

	*/

	bvgraph_close(&g);

	return 0;
}
