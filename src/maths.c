#include "lib.h"

void
v3_print(struct v3 * v3)
{
  printf("{");
  for (size_t i=0; i<2; i++) {
    printf("%f,", v3->a[i]);
  }
  printf("%f}\n", v3->a[2]);
}

void
v4_print(struct v4 * v4)
{
  printf("{");
  for (size_t i=0; i<3; i++) {
    printf("%f,", v4->a[i]);
  }
  printf("%f}\n", v4->a[3]);
}

void
m4_print(struct m4 * m4)
{
  printf("{");
  for (size_t i=0; i<4; i++) {
    if (i>0) {
      printf(" ");
    }
    for (size_t j=0; j<4; j++) {
      if (i == 3 && j == 3) {
        printf("%f}\n", m4->m[i][j]);
      } else {
        printf("%f,", m4->m[i][j]);
      }
    }
    printf("\n");
  }
}
