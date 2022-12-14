#include <stdio.h>

int rand( short seed ){
  unsigned short _MASK = 0xB400u;

  short start_seed = seed;
  int period = 0;

  do {
    short otpt = seed & 1;
    if( otpt ){
      seed ^= _MASK;
    }
    seed >>= 1;
    printf("%d%s", otpt, period % 16 == 0? "\n": "" );

    period++;
  } while( seed != start_seed && seed != 0 );
  printf("\n");

  return period;

}

int main( int argc, char* argv[] ){
  int prd = rand(0xe);
  printf("prd = %d\n", prd);

  return 0;
}
