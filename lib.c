#include "lib.h"

bool key_down[NUM_KEYS_DOWN] = {0};
/* Initialize event-queue. */
struct event_queue event_queue = {
  .current = event_queue.queue,
  .last= event_queue.queue,
};

void
error(const char * fmt, ...) {
  va_list ap;
  fprintf(stderr, "[!]: ");
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}

void
info(const char * fmt, ...) {
  va_list ap;
  fprintf(stdout, "[-]: ");
  va_start(ap, fmt);
  vfprintf(stdout, fmt, ap);
  va_end(ap);
}

void
event_queue_add(int key, bool pressed, bool released)
{
  if (event_queue.last+1 == event_queue.current) {
    /* Last looped around and is behind current, queue full. */
  }
}
