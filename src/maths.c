#include "lib.h"

void
v3_print(struct v3 * v3)
{
  printf("{");
  for (size_t i=0; i<2; i++) {
    printf("%.2f,", v3->a[i]);
  }
  printf("%.2f}\n", v3->a[2]);
}

void
v4_print(struct v4 * v4)
{
  printf("{");
  for (size_t i=0; i<3; i++) {
    printf("%.2f,", v4->a[i]);
  }
  printf("%.2f}\n", v4->a[3]);
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
        printf("%.2f}\n", m4->m[i][j]);
      } else {
        printf("%.2f,", m4->m[i][j]);
      }
    }
    printf("\n");
  }
}

struct v3
v3_sub_v3(struct v3 * v1, struct v3 * v2)
{
  return (struct v3){{{
    v1->x - v2->x,
    v1->y - v2->y,
    v1->z - v2->z,
  }}};
}

struct v3
v3_normalize(struct v3 * v)
{
  size_t sum = abs(v->x)+abs(v->y)+abs(v->z);
  return (struct v3){{{
    v->x/sum,
    v->y/sum,
    v->z/sum,
  }}};
}

struct v3
v3_sub_v3_norm(struct v3 * v1, struct v3 * v2)
{
  struct v3 temp = v3_sub_v3(v1, v2);
  return v3_normalize(&temp);
}
