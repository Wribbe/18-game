#ifndef LIB_H
#define LIB_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#define NUM_KEYS_DOWN 512
#define SIZE_EVENT_QUEUE 512

/* Logging functions.
 * ------------------ */
void
error(const char * fmt, ...);

void
info(const char * fmt, ...);

/* Structs
 * ------- */

struct event_key_action {
  bool pressed;
  bool released;
  int key;
};

struct event_queue {
  struct event_key_action queue[SIZE_EVENT_QUEUE];
  struct event_key_action * current;
  struct event_key_action * last;
};

/* External definitions.
 * --------------------- */

extern bool key_down[NUM_KEYS_DOWN];
extern struct event_queue event_queue;

/* Lib functions.
 * -------------- */

void
event_queue_add(int key, bool pressed, bool released);

void
event_queue_parse(void);

#endif
