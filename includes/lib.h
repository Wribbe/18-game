#ifndef LIB_H
#define LIB_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <unistd.h>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#define NUM_KEYS_DOWN 512
#define SIZE_EVENT_QUEUE 512

/* Macros
 * ------ */
#define UNUSED(x) (void)x;

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

/* External definitions.
 * --------------------- */

extern GLFWwindow * current_window;

/* Lib functions.
 * -------------- */

GLFWwindow *
init_window(size_t width, size_t height, const char * title);

/* handling_keys.c */
void
event_queue_process(void);

void
key_callback(GLFWwindow * window, int key, int scancode, int action, int mods);

#endif
