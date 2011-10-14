/** 
 * @file refill_test.c
 * Test the refill function in bitfile, which is internal,
 * so we can't test it with a pure interface
 */

#include "bitfile.h"
#include "bitfile.c"

int main(void) {
  bitfile bfmem = {0};
  bitfile *bf = &bfmem;
  
  // setup a 1-byte memory file
  unsigned char mem1 = 8;
  bitfile_map(&mem1, 1, bf);
  // check to make sure there is no fill
  assert(bf->fill == 0);
  refill16(bf);
  // check 
  assert(bf->fill == 8);
  while (bf->fill > 0) {
    int bit = read_from_current(bf, 1);
    printf("%i", bit);
  }
  printf("\n");
  
  unsigned char mem3[3] = {128+53, 8, 9};
  bitfile_map(mem3, 3, bf);
  assert(bf->fill == 0);
  
  int firstbit = read_from_current(bf, 1); // this will only load 8-bits in
  assert(bf->fill == 7); // and we've used one
  assert(firstbit == 1);    
  assert(refill16(bf) == 23);
  assert(bf->fill == 23);
  int nextbits = read_from_current(bf, 7);
  assert(nextbits == 53);
  nextbits = read_from_current(bf, 4);
  assert(nextbits == 0);
  assert(bf->fill == 12);
  assert(refill16(bf) == 12);
  
  
  
  
  return 0;
}