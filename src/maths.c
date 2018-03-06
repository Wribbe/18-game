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
v3_sub(struct v3 * v1, struct v3 * v2)
{
  return (struct v3){{{
    v1->x - v2->x,
    v1->y - v2->y,
    v1->z - v2->z,
  }}};
}

struct v3
v3_add(struct v3 * v1, struct v3 * v2)
{
  return (struct v3){{{
    v1->x + v2->x,
    v1->y + v2->y,
    v1->z + v2->z,
  }}};
}

struct v3
v3_mul(struct v3 * v1, struct v3 * v2)
{
  return (struct v3){{{
    v1->x * v2->x,
    v1->y * v2->y,
    v1->z * v2->z,
  }}};
}

struct v3
v3_mulf(float f, struct v3 * v)
{
  return (struct v3){{{
    v->x *= f,
    v->y *= f,
    v->z *= f,
  }}};
}

struct v3
v3_invert(struct v3 * v)
{
  return (struct v3){{{
    v->x *= -1,
    v->y *= -1,
    v->z *= -1,
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
v3_cross(struct v3 * a, struct v3 * b)
{
  return (struct v3) {{{
    a->y*b->z - a->z*b->y,
    a->z*b->x - a->x*b->z,
    a->x*b->y - a->y*b->x,
  }}};
}

struct v4
m4_mul_v4(struct m4 * m, struct v4 * v)
{
  struct v4 r = {0};
  for (size_t i = 0; i<4; i++) {
    for (size_t j = 0; j<4; j++) {
      r.a[i] += m->m[i][j] * v->a[j];
    }
  }
  return r;
}

struct m4
m4_perspective(GLfloat fov_y, GLfloat aspect, GLfloat near, GLfloat far)
{
  struct m4 ret = {0};
  GLfloat a = 1.0f/tanf(fov_y/2.0f);

  ret.m[0][0] = a/aspect;
  ret.m[1][1] = a;
  ret.m[2][2] = -(far + near)/(far-near);
  ret.m[2][3] = (-2*far*near)/(far-near);
  ret.m[3][2] = -1;
  return ret;
}

struct m4
m4_mul(struct m4 * m1, struct m4 * m2)
{
  struct m4 r = {0};
  for (size_t i = 0; i<4; i++) {
    for (size_t j = 0; j<4; j++) {
      for (size_t k = 0; k<4; k++) {
        r.m[i][j] += m1->m[i][k] * m2->m[k][j];
      }
    }
  }
  return r;
}

struct m4
m4_mul3(struct m4 * m1, struct m4 * m2, struct m4 * m3)
{
  struct m4 temp = m4_mul(m2, m3);
  return m4_mul(m1, &temp);
}
