
/* common testing functions */

#include <sys/time.h>
double get_wall_time(){
    struct timeval time;
    if (gettimeofday(&time,NULL)){
        //  Handle error
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}
 
uint64_t xorseed=1; /* The state must be seeded with a nonzero value. */
 
uint64_t xorshift64star(void) {
	xorseed ^= xorseed >> 12; // a
    xorseed ^= xorseed << 25; // b
    xorseed ^= xorseed >> 27; // c
	return xorseed * UINT64_C(2685821657736338717);
}