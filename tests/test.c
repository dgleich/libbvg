#include <stdio.h>
#include <stdlib.h>
#include "../include/bvgraph.h"
#include "../include/bvgraphfun.h"
#include "../include/bitfile.h"
#include <string.h>
#include <time.h>

void test_iteratively(bvgraph g, bvgraph iterate_g){
	bvgraph_iterator git;
	bvgraph_nonzero_iterator(&iterate_g, &git);

	for (; bvgraph_iterator_valid(&git); bvgraph_iterator_next(&git)) {
		int *iterate_links, *random_links;
		unsigned int iterate_d, random_d;

		printf("Loading node %d by non-zero iterator...\n", git.curr);
		bvgraph_iterator_outedges(&git, &iterate_links, &iterate_d);
		printf("Loading node %d by random access...\n", git.curr);
		bvgraph_successors(&g, git.curr, &random_links, &random_d);

		printf("Checking degree... ");
		if (iterate_d == random_d){
			printf("Degree correct.\n");
		}
		else{
			printf("Degree wrong. Stop.\n");
			return;
		}

		printf("Checking links... ");
		int i = 0;
		for (i; i<iterate_d; i++){
			if (iterate_links[i] != random_links[i]){
				printf("Link %d doesn't match. Stop.\n", i);
				return;
			}
		}
		printf("Links correct. Node %d checking done.\n", git.curr);
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

void print_all(bvgraph g){
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

void print_help(){
	printf("Use the following parameters to test random access: \n");
	printf("./a.out dataset param\n");
	printf("dataset: the graph dataset without extenion. Ex: harvard500\n");
	printf("param:\n");
	printf("\titerate   - to test iteratively.\n");
	printf("\trandom    - to test by randomly generated nodes. Need a parameter for # of nodes.\n");
	printf("\thead-tail - to test from head and tail roundly.\n");
	printf("\tprint     - print degree and links for all nodes in dataset\n");	
}

int main(int argc, char** argv){
	bvgraph g = {0};
	//const char* name = "libbvg/data/wb-cs.stanford";
	const char* name = argv[1];
	char* method = argv[2];

	if (method == NULL){
		print_help();
		return 0;
	}

	//load with offsets
	bvgraph_load(&g, name, strlen(name), 1);

	printf("Input file: %s\n", name);
	printf("nodes = %d\n", g.n);
	printf("edges = %lli\n", g.m);

	if (strcmp(method, "iterate") == 0){
		bvgraph iterate_g;
		bvgraph_load(&iterate_g, name, strlen(name), 0);
		test_iteratively(g, iterate_g);
	}
	else if (strcmp(method, "random") == 0){
		if (argv[3] == NULL){
			printf("Need node number. Stop\n");
			return 1;
		}
		int num = atoi(argv[3]);
		random_test(g, num);
	}
	else if (strcmp(method, "head-tail") == 0){
		head_tail_first_test(g);
	}
	else if (strcmp(method, "print") == 0){
		print_all(g);
	}
	else{
		print_help();
	}

	bvgraph_close(&g);

	return 0;
}
