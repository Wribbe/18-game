#include "lib.h"

void
create_cube(struct v3 * p1, struct v3 * p2, struct v3 * p3)
{
  v3_print(p1);
  v3_print(p2);
  v3_print(p3);
}

GLint
object_translate(GLuint id, struct v3 * v)
{
  if (id <= 0 || id > last_render_object) {
    return -1;
  }
  struct render_object * obj = &render_queue[id];
  obj->m4_model.m[0][3] += v->x;
  obj->m4_model.m[1][3] += v->y;
  obj->m4_model.m[2][3] += v->z;

  /* Perturb the bounding coordinates by same amount. */
  obj->bound_points.top_left = v3_add(&obj->bound_points.top_left, v);
  obj->bound_points.bottom_right = v3_add(&obj->bound_points.bottom_right, v);
  return 0;
}
