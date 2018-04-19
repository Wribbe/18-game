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

  if (INVALID_OBJECT_ID(id)) {
    error("object_translate received invalid id: %u.\n", id);
    return -1;
  }

  //TODO: Figure out why the x-axis needs to be inverted to translate correctly.
  struct render_object * obj = &render_queue[id];
  obj->m4_model.m[0][3] += -v->x;
  obj->m4_model.m[1][3] += v->y;
  obj->m4_model.m[2][3] += v->z;

  return 0;
}
