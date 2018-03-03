#ifndef LIB_H
#define LIB_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#define NUM_KEYS_DOWN 512
#define SIZE_EVENT_QUEUE 512

/* Macros
 * ------ */
#define UNUSED(x) (void)x;

/* Type-definitions
 * ---------------- */

typedef GLfloat v3[3];
typedef GLfloat v4[4];
typedef GLfloat m4[4][4];

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

/* External definitions.
 * --------------------- */

extern GLFWwindow * current_window;

/* Lib functions.
 * -------------- */

GLFWwindow *
init_window(size_t width, size_t height, const char * title);

char *
file_read(const char * filepath);

GLfloat *
file_read_floats(const char * filepath, size_t * num_floats);

GLuint
shader_program_create(const char * path_vertex, const char * path_fragment);

struct vao
buffer_create(GLfloat * floats, size_t num_floats);

void
draw_arrays(GLenum type, struct vao * vao);

/* handling_keys.c */
void
event_queue_process(void);

void
key_callback(GLFWwindow * window, int key, int scancode, int action, int mods);

/* maths.c */

void
v3_print(v3 * v3);

void
v4_print(v4 * v4);

void
m4_print(m4 * m4);


#endif
