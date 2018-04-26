#include "lib.h"

void
create_cube(struct v3 * p1, struct v3 * p2, struct v3 * p3)
{
  v3_print(p1);
  v3_print(p2);
  v3_print(p3);
}

void
object_translate(GLuint id, struct v3 * v)
{
  struct render_object * obj = get_render_object(id);
  obj->m4_model_delta.m[0][3] += v->x;
  obj->m4_model_delta.m[1][3] += v->y;
  obj->m4_model_delta.m[2][3] += v->z;
}

void
object_scale(GLuint id, struct v3 * v)
{
  struct render_object * obj = get_render_object(id);
  obj->m4_model_delta.m[0][0] *= v->x;
  obj->m4_model_delta.m[1][1] *= v->y;
  obj->m4_model_delta.m[2][2] *= v->z;
}

void
object_add_force(GLuint id, struct v3 * v)
{
  struct render_object * obj = get_render_object(id);
  obj->state.force = v3_add(&obj->state.force, v);
}
