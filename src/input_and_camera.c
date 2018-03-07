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

/* Set-up globals for camera positions and transformation matrices. */
struct v3 camera_up;

struct v3 camera_position;
struct v3 camera_target;
struct v3 camera_direction;
struct v3 camera_front;
struct v3 camera_right;

struct m4 m4_model;
struct m4 m4_view;
struct m4 m4_projection;
struct m4 m4_mvp;

/* Global numerical values. */
GLfloat camera_speed = 1.5f;
double time_prev = 0;
double time_delta = 0;
double mouse_x = 0;
double mouse_y = 0;

/* Global event booleans. */
bool b_mvp_recalculate = false;
bool b_camera_changed = false;

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

/* Camera functions.
 * ----------------- */

void
m4_mvp_calculate(void)
{
  m4_mvp = m4_mul3(&m4_projection, &m4_view, &m4_model);
  program_bind_mat4fv(current_shader_program, UNIFORM_NAME_MVP, &m4_mvp);
}

void
camera_look_at(struct v3 * target)
{
  b_camera_changed = true;
  /* Calculate new direction-vector. */
  struct v3 d = v3_sub(&camera_position, target);
  d = v3_normalize(&d);
  /* Calculate new right-vector. */
  struct v3 r = v3_cross(&d, &camera_up);
  r = v3_normalize(&r);

  /* Calculate new up-vector. */
  struct v3 u = v3_cross(&r, &d);

  /* Update the view matrix.
   *
   *  Note: Calculation of the direction vector results in a vector that points
   *  in the wrong direction:
   *
   *  direction-vector | (target - camera)
   *                   v
   *        [target] .<---. >[] [camera]
   *                 |   /
   *                 |  /
   *                 | /
   *                 |/
   *                 x [origo]
   *
   * */
  m4_view = (struct m4){{
    { r.x,  r.y,  r.z, -v3_dot(&r, &camera_position)},
    { u.x,  u.y,  u.z, -v3_dot(&u, &camera_position)},
    { d.x,  d.y,  d.z,  v3_dot(&d, &camera_position)},
    {   0,    0,    0,  1},
  }};

  /* Update new camera vectors. */
  camera_direction = v3_invert(&d);
  camera_up = u;
  camera_right = r;
}

struct v3
camera_forward(void)
{
  double camera_speed_delta = camera_speed * time_delta;
  struct v3 camera_move = v3_mulf(camera_speed_delta, &camera_front);
  b_camera_changed = true;
  return v3_add(&camera_position, &camera_move);
}

struct v3
camera_backwards(void)
{
  double camera_speed_delta = camera_speed * time_delta;
  struct v3 camera_move = v3_mulf(camera_speed_delta, &camera_front);
  b_camera_changed = true;
  return v3_sub(&camera_position, &camera_move);
}

void
camera_system_init(void)
{
  camera_position = (struct v3){{{0.0f, 0.0f, -3.0f}}};
  camera_target = (struct v3){{{0.0f, 0.0f, 0.0f}}};
  camera_up = (struct v3){{{0.0f, 1.0f, 0.0f}}};

  camera_direction = v3_sub(&camera_position, &camera_target);
  camera_direction = v3_normalize(&camera_direction);

  camera_right = v3_cross(&camera_up, &camera_direction);
  camera_right = v3_normalize(&camera_right);

  camera_front = (struct v3){{{0.0f, 0.0f, 1.0f}}};

  m4_model = m4_identity();
  m4_model.m[0][3] = -2;
  m4_view = m4_identity();

  m4_projection = m4_perspective(M_PI/2, 1000.0f/600.0f, 0.1f, 100.0f);
  m4_mvp_calculate();

  camera_look_at(&camera_target);
}

void
camera_position_propagate(void)
{
  m4_view.m[0][3] = camera_position.x;
  m4_view.m[1][3] = camera_position.y;
  m4_view.m[2][3] = camera_position.z;
}

/* Timing related functions.
 * ------------------------- */

void
clock_init(void)
{
  time_prev = glfwGetTime();
}

void
clock_tick(void)
{
  double time_current = glfwGetTime();
  time_delta = time_current-time_prev;
  time_prev = time_current;
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
  }
  if (key_down_single(GLFW_KEY_SPACE)) {
  }
  if (key_down(GLFW_KEY_SPACE)) {
    if (key_down(GLFW_KEY_LEFT_SHIFT)) {
      camera_position = camera_backwards();
    } else {
      camera_position = camera_forward();
    }
  }
}

void
globals_event_reset(void)
{
  b_mvp_recalculate = false;
  b_camera_changed = false;
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
  /* Anything needs recalculating? */
  if (b_camera_changed) {
    camera_position_propagate();
    b_mvp_recalculate = true;
  }
  if (b_mvp_recalculate) {
    m4_mvp_calculate();
  }
  /* Reset all event globals. */
  globals_event_reset();
}

/* GLFW key-callback functions.
 * ---------------------------- */

void
callback_key(GLFWwindow * window, int key, int scancode, int action, int mods)
{
  UNUSED(window); UNUSED(scancode); UNUSED(mods);
  if (action == GLFW_REPEAT) {
    return;
  }
  event_queue_add(key, action == GLFW_PRESS ? true : false);
}

void
callback_mouse_position(GLFWwindow * window, double pos_x, double pos_y)
{
  UNUSED(window);
  mouse_x = pos_x;
  mouse_y = pos_y;
}
