#ifndef LIB_H
#define LIB_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

/* Definitions
 * ----------- */

#define NUM_KEYS_DOWN 512
#define SIZE_EVENT_QUEUE 512
#define M_PI 3.14159265358979323846
#define UNIFORM_NAME_MVP "mvp"
#define UNIFORM_NAME_COLOR_BORDER "color_border"
#define NUM_RENDER_OBJECTS 512

enum obj_state {
  OBJ_STATE_COLLISION,
};

/* Macros
 * ------ */
#define UNUSED(x) (void)x
#define COUNT(x) ((sizeof(x)/sizeof(x[0])))
#define INVALID_OBJECT_ID(id) (id <= 0 || id > last_render_object)

/* Logging functions.
 * ------------------ */
void
error(const char * fmt, ...);

void
info(const char * fmt, ...);

/* Structs
 * ------- */

struct key_action {
  bool pressed;
  int key;
};

struct event_queue {
  struct key_action queue[SIZE_EVENT_QUEUE];
  struct key_action * current;
  struct key_action * last;
};

struct key_status {
  bool pressed;
  bool change_to_process;
};

struct vao {
  GLuint id;
  size_t stride;
  size_t num_indices;
};

struct v3 {
  union {
    GLfloat a[3];
    struct {
      GLfloat x;
      GLfloat y;
      GLfloat z;
    };
  };
};

struct v4 {
  union {
    GLfloat a[4];
    struct {
      GLfloat x;
      GLfloat y;
      GLfloat z;
      GLfloat w;
    };
  };
};

struct m4 {
  GLfloat m[4][4];
};

struct bound_points {
  struct v3 top_left;
  struct v3 bottom_right;
};

struct state {
  bool colliding;
  struct v3 force;
};

struct bound_square {
  struct v3 points[4];
};

struct render_object {
  bool active;
  GLuint render_type;
  struct vao vao;
  struct m4 m4_model;
  struct bound_points bound_points;
  struct bound_square bound_square;
  struct bound_square bound_square_model;
  struct state state;
};

struct side {
  struct v3 p[2];
  struct v3 normal;
};

/* External definitions.
 * --------------------- */

extern GLFWwindow * current_window;
extern GLuint program_shader_debug;
extern GLuint program_shader_default;
extern struct m4 m4_mvp;
extern struct m4 m4_view;
extern struct m4 m4_projection;
extern double delta_time;
extern struct render_object render_queue[NUM_RENDER_OBJECTS];
extern GLuint last_render_object;
extern GLuint vao_debug;
extern GLuint id_object_player;


/* DEBUG globals
 * ----------------- */

extern bool b_debug_draw_bounding_squares;

/* Lib functions.
 * -------------- */

GLFWwindow *
init_window(size_t width, size_t height, const char * title);

char *
file_read(const char * filepath);

GLfloat *
file_read_floats(const char * filepath, size_t * num_floats);

GLuint
program_shader_create(const char * path_vertex, const char * path_fragment);

struct vao
buffer_create(GLfloat * floats, size_t num_floats);

void
draw_objects(void);

GLuint
render_object_create(GLfloat * floats, size_t num_floats);

void
program_use(GLuint id_program);

void
program_bind_mat4fv(GLuint id_program, const char * uniform, struct m4 * data);

void
program_bind_3fv(GLuint id_program, const char * uniform, struct v3 * data);

void
init_environment(void);

void
physics_tick(void);

void
set_as_player(GLuint object_id);

struct render_object *
get_render_object(GLuint id);

void
debug_program_set_border_color(struct v3 * color);

/* input_and_camera.c */

void
event_queue_process(void);

void
callback_key(GLFWwindow * window, int key, int scancode, int action, int mods);

void
callback_cursor_position(GLFWwindow * window, double pos_x, double pos_y);

void
callback_mouse_key(GLFWwindow * window, int button, int action, int mods);

void
camera_system_init(void);

void
clock_init(void);

void
clock_tick(void);

struct m4
m4_mvp_calculate(struct m4 * model);

/* maths.c */

void
v3_print(struct v3 * v3);

void
v4_print(struct v4 * v4);

void
m4_print(struct m4 * m4);

struct v3
v3_sub(struct v3 * v1, struct v3 * v2);

struct v3
v3_add(struct v3 * v1, struct v3 * v2);

struct v3
v3_mul(struct v3 * v1, struct v3 * v2);

GLfloat
v3_dot(struct v3 * v1, struct v3 * v2);

struct v3
v3_mulf(float f, struct v3 * v);

struct v3
v3_normalize(struct v3 * v);

struct v3
v3_invert(struct v3 * v);

struct v3
v3_cross(struct v3 * a, struct v3 * b);

struct v3
m4_mul_v3(struct m4 * m, struct v3 * v);

struct v4
m4_mul_v4(struct m4 * m, struct v4 * v);

struct m4
m4_perspective(GLfloat fov_y, GLfloat aspect, GLfloat near, GLfloat far);

struct m4
m4_mul(struct m4 * m1, struct m4 * m2);

struct m4
m4_mul3(struct m4 * m1, struct m4 * m2, struct m4 * m3);

struct m4
m4_identity(void);

/* object_create.c */

void
create_cube(struct v3 * p1, struct v3 * p2, struct v3 * p3);

void
object_translate(GLuint id, struct v3 * v);

void
object_scale(GLuint id, struct v3 * v);

void
object_add_force(GLuint id, struct v3 * v);

#endif
