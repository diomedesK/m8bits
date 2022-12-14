#include <stdlib.h>
#include <stdio.h>

typedef unsigned char byte;
typedef struct {
  byte X;
  byte Y;
} Enemy;

Enemy enemies[10];

int main(int argc, char *argv[])
{

  char *vect[] = {
    "Antarctica",
    "1000Blunts",
    "LilKennedy",
    "California"
  };

  for (int i = 0; i < 4; ++i) {
    char *str = vect[i];
    printf("%d ||| vect - str = %d\n", i, &vect[3] - &vect[i]  );

  }



  return 0;
}
