/* 
 * David Gleich
 * Copyright, Stanford University, 2007
 * 17 May 2007
 */

/**
 * @file bvgraph_test.c
 * Read a bvgraph from a file and write its data to stdout
 */

#include "bvgraph.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>

// disable all of the unsafe operation warnings
#ifdef _MSC_VER
#define inline __inline
#if _MSC_VER >= 1400
#pragma warning ( push )
#pragma warning ( disable: 4996 )
#endif /* _MSC_VER >= 1400 */
#endif /* _MSC_VER */

#define MAXDEG 80000
#define N 97776665 //// 64  97776665  6257706595
#define NODES 6257706595
              
long long discover[N] = {0}; // use bit array
struct Node{
	long long val;
	struct Node *next;
};
struct queue{
	struct Node *head; // head points to q head
	struct Node *tail; // tail points to the last element
	long long size;
};

int initQ(struct queue *q);
int emptyQ(struct queue *q);
int enqueue(struct queue *q, long long elem);
long long dequeue(struct queue *q);
int setBit(long long node);
int checkBit(long long node);
int bfs_search(char *filename, long long begin);
int degree_dist(bvgraph *g, char *filename, int filenamelen);

int main(int argc, char **argv) 
{
    bvgraph graph = {0};
    bvgraph *g = &graph;

    char *filename;
    int filenamelen;

    int i;

    if (argc < 2) { fprintf(stderr, "Usage: bvgraph_test bvgraph_basename\n"); return (-1); }

    filename = argv[1];
    filenamelen = (int)strlen(filename);
	degree_dist(g, filename, filenamelen);
	for (i = 0; i < 10; i ++){
		srand( time(NULL) );
		long long begin = rand() % NODES;
		printf("perform bfs start from %llu\n", begin);
		bfs_search(filename, begin);
	}
	
   /* rval = bvgraph_load(g, filename, filenamelen, -1);
    if (rval) { perror("error with initial load!"); }
    {
        size_t memrequired, offset_buff;
        bvgraph_required_memory(g, 1, &memrequired, &offset_buff);
        printf("the graph %s requires %llu bytes to load into memory, offset_buff=%llu\n", filename, (long long)memrequired, (long long)offset_buff);
    }
    bvgraph_close(g);

    //rval = bvgraph_load(g, filename, filenamelen, 0);
	rval = bvgraph_load(g, filename, filenamelen, 1);
	printf("#nodes = %llu, #edges = %llu, rval=%d\n", g->n, g->m, rval);
    if (rval) { perror("error with full load!"); }
    {
        bvgraph_iterator iter;
		bvgraph_random_iterator riter;
        long long *links;
        unsigned long long d;
		int deg[MAXDEG] = {0};
		i = 0;
		FILE *f = fopen("random2.txt","w");
		if (f == NULL){
			printf("Error opening file!\n");
			exit(1);
		}
		bvgraph_random_access_iterator(g, &riter);
		printf("after bvgraph_random_access_iterator\n");
		i = 999;
		bvgraph_random_outdegree(&riter, i, &d);
		printf("there are %llu neighbors for node %llu\n", d, i);
		printf("after bvgraph_random_outdegree\n");
		long long * start = NULL;
		bvgraph_random_successors(&riter, i, &start, &d);
		printf("after bvgraph_random_successors\n");
		long long j;
		for (j = 0; j < d; j ++){
			fprintf(f, "%llu -> %llu\n", i, start[j]);
		}
		fclose(f);
		printf("random access done!!!\n");*/
        // initialize a vector of column sums
        //long long *colsum = malloc(sizeof(long long)*g->n);
        //long long *colsum2 = malloc(sizeof(long long)*g->n);
        //int rep;
        //memset(colsum, 0, sizeof(long long)*g->n);
		//printf("computing degree!!!\n");
        /*for (bvgraph_nonzero_iterator(g, &iter); 
             bvgraph_iterator_valid(&iter); 
             bvgraph_iterator_next(&iter))
        {
            bvgraph_iterator_outedges(&iter, &links, &d);
			if (d > MAXDEG){
				printf("d is %llu\n", d);
				exit(1);
			}
			deg[d] ++;
			i ++;
			if (i % 1000000 == 0){
				printf("#nodes = %llu\n", i);
			}
			if (i == 1000){
            	for (i = 0; i < d; i++) {
					fprintf(f, "%llu -> %llu\n", iter.curr, links[i]);
                //colsum[links[i]]++;
				}
				printf("Random access write done\n");
				break;
			}
        }*/
		//bvgraph_random_free(&riter);
        //bvgraph_iterator_free(&iter);
		
		//for (i = 0; i < MAXDEG; i ++){
		//	fprintf(f, "%d\n", deg[i]);
		//}
		//printf("Writing deg file done!\n");
		
       /* for (rep = 0; rep < 10000; rep++) {
            //memset(colsum2, 0, sizeof(int)*g->n);
            for (bvgraph_nonzero_iterator(g, &iter); 
                 bvgraph_iterator_valid(&iter); 
                 bvgraph_iterator_next(&iter))
            {
            //    bvgraph_iterator_outedges(&iter, &links, &d);
                for (i = 0; i < d; i++) {
                    colsum2[links[i]]++;
                }
            }
            bvgraph_iterator_free(&iter);
            for (i=0; i < g->n; i++) {
                if (colsum2[i] != colsum[i]) {
                    fprintf(stderr, "error, column sum of column %llu is not correct (%llu =? %llu)", 
                        i, colsum[i], colsum2[i]);
                    perror("colsum error!");
                }
            }
        }*/
        //free(colsum);
        //free(colsum2);
    //}
   // bvgraph_close(g);

   /* for (i = 0; i < 10000000; i++) {
        rval = bvgraph_load(g, filename, filenamelen, 0);
        bvgraph_close(g);
    }*/
	return 0;
}

int initQ(struct queue *q){
	q->size = 0;
	q->head = NULL;
	q->tail = NULL;
	return 0;
}

int emptyQ(struct queue *q){
	if (q->size == 0){
		return 1;
	}
	else{
		return 0;
	}
}

int enqueue(struct queue *q, long long elem){
	struct Node *p = (struct Node*)malloc(sizeof(struct Node));
	if (p == NULL){
		return -1;
	}
	p->val = elem;
	p->next = NULL;
	if (q->size == 0){
		q->head = p;
		q->tail = p;
		q->size ++;
		return 0;
	}
	q->tail->next = p;
	q->tail = p;
	q->size ++;
	return 0;
}

long long dequeue(struct queue *q){
	long long rval;
	struct Node* headnext = NULL;
	if (q->size == 0){
		return -1;
	}
	else{
		/*if (q->head != NULL){
			printf("q->head->val=%llu\n", q->head->val);
		}
		else{
			printf("null\n");
		}*/
		rval = q->head->val;
		//printf("in dequeue: rval=%llu\n", rval);
		headnext = q->head->next;
		free(q->head);
		//printf("after free head\n");
		q->head = headnext;
		q->size --;
		return rval;
	}
}

int setBit(long long node){
	long long ind = node / 64;
	int offset = node % 64;
	discover[ind] = discover[ind] | (1 << offset);
	return 0;
}

int checkBit(long long node){
	long long ind = node / 64;
	int offset = node % 64;
	int rval = discover[ind] & (1 << offset);
	return rval;
}

int bfs_search(char *filename, long long begin)
{
	bvgraph graph = {0};
	bvgraph *g = &graph;
	struct queue q;
	int filenamelen;
	int rval;
	long long i;
	long long count = 0;
	
	filenamelen = (int)strlen(filename);
	rval = bvgraph_load(g, filename, filenamelen, 1);
	printf("#nodes = %llu, #edges = %llu, rval=%d\n", g->n, g->m, rval);
	if (rval) {perror("error with full load!");}
	{
		printf("Starting BFS on the graph ...\n");
		initQ(&q);
		
		unsigned long long d;
		
		enqueue(&q, begin);
		setBit(begin);
		
		while (!emptyQ(&q)){
			long long cur;
			long long *suc = NULL;
			bvgraph_random_iterator riter;
			//printf("before dequeue\n");
			cur = dequeue(&q);
			//printf("dequeue successful\n");
			if (cur < 0){
				printf("dequeue error!\n");
			}
			//printf("cur = %llu, qsize=%llu \n", cur, q.size);
			setBit(cur);
			bvgraph_random_access_iterator(g, &riter);
			bvgraph_random_successors(&riter, cur, &suc, &d);
			//printf("%llu has %llu successors\n", cur, d);
			count ++;
			if (count % 100000000 == 0){
				printf("%llu nodes scanned...\n", count);
			}
			for (i = 0; i < d; i ++){
				//printf("suc[i]=%llu\n", suc[i]);
				if (!checkBit(suc[i])){
					enqueue(&q, suc[i]);
					setBit(suc[i]);
				}
				else{
					continue;
				}				
			}
			//printf("put in queue done for %llu\n", cur);
			bvgraph_random_free(&riter);
			//printf("random_free done!\n");
			//printf("qsize=%llu\n", q.size);
		}
		
		bvgraph_close(g);
		printf("Found 1 component starting from %llu\n", begin);
		/*FILE *f = fopen("component.txt", "w");
		if (f == NULL){
			printf("Error opening file \n");
			exit(1);
		}

		for (i = 0; i < NODES; i ++){
			if (checkBit(i)){
				fprintf(f, "%llu\n", i);
			}
		}
		fclose(f);
		printf("writing component.txt done!\n");*/
	}
	return 0;
}

int degree_dist(bvgraph *g, char *filename, int filenamelen){
	int rval;
	
	printf("Loading graph (without whole graph into memory) ...\n");
	rval = bvgraph_load(g, filename, filenamelen, -1);
	if (rval) { perror("error with initial load!"); }
	{
		printf("#nodes = %llu, #edges = %llu, rval=%d\n", g->n, g->m, rval);
		bvgraph_iterator iter;
		long long *links;
		unsigned long long d, i;
		int deg[MAXDEG] = {0};
		i = 0;
		for (bvgraph_nonzero_iterator(g, &iter);
			bvgraph_iterator_valid(&iter);
			bvgraph_iterator_next(&iter))
		{
			bvgraph_iterator_outedges(&iter, &links, &d);
			if (d > MAXDEG){
				printf("detecting a larger degree %llu\n", d);
				exit(1);
			}
			deg[d] ++;
			i ++; 
			if (i % 100000000 == 0){
				printf("#nodes = %llu scanned for deg_dist()\n", i);
			}
		}
		bvgraph_iterator_free(&iter);
	}
	bvgraph_close(g);
	printf("deg_dist() done!\n");
	return 0;
}