#include "lib.h"

/* Definition of extern globals.
 * ----------------------------- */
struct key_status key_status[NUM_KEYS_DOWN] = {0};
/* Initialize event-queue. */
struct event_queue struct_event_queue = {
  .current = struct_event_queue.queue,
  .last = struct_event_queue.queue+SIZE_EVENT_QUEUE,
};
/* Set up pointer to struct_event_queue. */
struct event_queue * event_queue = &struct_event_queue;
/* Set global current_window to NULL. */
GLFWwindow * current_window = NULL;

/* Key status functions.
 * --------------------- */
bool
key_down(int key)
{
  return key_status[key].pressed;
}

bool
key_not_processed(int key)
{
  if (key_status[key].change_to_process) {
    key_status[key].change_to_process = false;
    return true;
  }
  return false;
}

bool
key_up_single(int key)
{
  return !key_down(key) && key_not_processed(key);
}

bool
key_down_single(int key)
{
  return key_down(key) && key_not_processed(key);
}

/* Event queue functions.
 * ---------------------- */

void
event_queue_add(int key, bool pressed)
{
  if (event_queue->current >= event_queue->last) {
    error("Event-queue full, event not added.\n");
    return;
  }

  /* Add event to queue. */
  event_queue->current->pressed = pressed;
  event_queue->current->key = key;

  /* Advance 'current' pointer. */
  event_queue->current++;
}

void
event_evalute_bindings(void)
{
  if (key_down(GLFW_KEY_ESCAPE)) {
    glfwSetWindowShouldClose(current_window, GLFW_TRUE);
  }
  if (key_down_single(GLFW_KEY_Q)) {
    if (key_down(GLFW_KEY_LEFT_SHIFT)) {
      info("Shift also pressed, don't close!\n");
    } else {
      glfwSetWindowShouldClose(current_window, GLFW_TRUE);
    }
  }
  if (key_up_single(GLFW_KEY_SPACE)) {
    printf("RELEASED SPACE!\n");
  }
  if (key_down_single(GLFW_KEY_SPACE)) {
    printf("SPACE PRESSED!\n");
  }
  if (key_down(GLFW_KEY_SPACE)) {
    printf("SPAAAAACE!\n");
  }
}

void
event_queue_process(void)
{
  /* Trigger glfw polling of events. */
  glfwPollEvents();
  /* Parse all actions in the queue. */
  struct key_action * action = event_queue->queue;
  for (;action < event_queue->current; action++) {
    key_status[action->key].change_to_process = true;
    key_status[action->key].pressed = action->pressed;
  }
  /* Set pointer to beginning of queue. */
  event_queue->current = event_queue->queue;
  /* Execute all matching key-bindings. */
  event_evalute_bindings();
}

/* GLFW key-callback function.
 * --------------------------- */

void
key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
  UNUSED(window); UNUSED(scancode); UNUSED(mods);
  if (action == GLFW_REPEAT) {
    return;
  }
  event_queue_add(key, action == GLFW_PRESS ? true : false);
}
