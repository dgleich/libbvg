
/* common testing functions */

#ifdef _WIN32
/* see 
 * http://stackoverflow.com/questions/17432502/how-can-i-measure-cpu-time-and-wall-clock-time-on-both-linux-windows
 */
#include <windows.h>
double get_wall_time(){
    LARGE_INTEGER time,freq;
    if (!QueryPerformanceFrequency(&freq)){
        //  Handle error
        return 0;
    }
    if (!QueryPerformanceCounter(&time)){
        //  Handle error
        return 0;
    }
    return (double)time.QuadPart / freq.QuadPart;
}
#else
#include <sys/time.h>
double get_wall_time(){
    struct timeval time;
    if (gettimeofday(&time,NULL)){
        //  Handle error
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}
#endif
 
uint64_t xorseed=1; /* The state must be seeded with a nonzero value. */
 
uint64_t xorshift64star(void) {
	xorseed ^= xorseed >> 12; // a
    xorseed ^= xorseed << 25; // b
    xorseed ^= xorseed >> 27; // c
	return xorseed * UINT64_C(2685821657736338717);
}