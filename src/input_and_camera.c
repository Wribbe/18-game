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
struct v3 direction_global_up;

struct v3 camera_position;
struct v3 camera_target;
struct v3 camera_direction;
struct v3 camera_right;

struct m4 m4_camera_model;
struct m4 m4_view;
struct m4 m4_projection;
struct m4 m4_mvp;

GLfloat camera_yaw;
GLfloat camera_pitch;
GLfloat camera_sensitivity;

/* Global numerical values. */
GLfloat camera_speed = 1.5f;
double time_prev = 0;
double time_delta = 0;
GLfloat cursor_x = 0;
GLfloat cursor_y = 0;
GLfloat cursor_prev_x = 0;
GLfloat cursor_prev_y = 0;

/* Global event booleans. */
bool b_mvp_recalculate = false;
bool b_view_recalculate = false;
bool b_camera_changed = false;
bool b_cursor_changed = false;
bool b_cursor_init = false;

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

struct m4
m4_mvp_calculate(struct m4 * m4_model)
{
  struct m4 m4_mvp = m4_mul3(&m4_projection, &m4_view, m4_model);
  program_bind_mat4fv(current_shader_program, UNIFORM_NAME_MVP, &m4_mvp);
  return m4_mvp;
}

void
camera_look_at(struct v3 * target)
{
  /* CURRENTLY NOT IN USE. */
  //TODO: Figure out why this flips the camera every frame.
  b_camera_changed = true;
  /* Calculate new direction-vector. */
  struct v3 new_camera_direction = v3_sub(&camera_position, target);
  new_camera_direction = v3_normalize(&new_camera_direction);
  /* Calculate new right-vector. */
  struct v3 new_camera_right = v3_cross(&camera_up, &new_camera_direction);
  new_camera_right = v3_normalize(&new_camera_right);

  /* Calculate new up-vector. */
  struct v3 new_camera_up = v3_cross(&new_camera_direction, &new_camera_right);

  /* Update the view matrix.
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

  /* Update new camera vectors. */
  camera_direction = new_camera_direction;
  camera_up = new_camera_up;
  camera_right = new_camera_right;

  b_view_recalculate = true;
}


void
m4_view_calculate(void)
{
  struct v3 r = camera_right;
  struct v3 u = camera_up;
  struct v3 d = camera_direction;
  struct v3 p = camera_position;

  m4_view = (struct m4){{
    { r.x,  r.y,  r.z, -v3_dot(&r, &p)},
    { u.x,  u.y,  u.z, -v3_dot(&u, &p)},
    { d.x,  d.y,  d.z, -v3_dot(&d, &p)},
    {   0,    0,    0,  1},
  }};
}

struct v3
camera_go_forward(void)
{
  double camera_speed_delta = camera_speed * time_delta;
  struct v3 camera_move = v3_mulf(camera_speed_delta, &camera_direction);
  b_camera_changed = true;
  return v3_sub(&camera_position, &camera_move);
}

struct v3
camera_go_backwards(void)
{
  double camera_speed_delta = camera_speed * time_delta;
  struct v3 camera_move = v3_mulf(camera_speed_delta, &camera_direction);
  b_camera_changed = true;
  return v3_add(&camera_position, &camera_move);
}

struct v3
camera_go_left(void)
{
  double camera_speed_delta = camera_speed * time_delta;
  struct v3 camera_move = v3_mulf(camera_speed_delta, &camera_right);
  b_camera_changed = true;
  return v3_sub(&camera_position, &camera_move);
}

struct v3
camera_go_right(void)
{
  double camera_speed_delta = camera_speed * time_delta;
  struct v3 camera_move = v3_mulf(camera_speed_delta, &camera_right);
  b_camera_changed = true;
  return v3_add(&camera_position, &camera_move);
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

  m4_camera_model = m4_identity();
  m4_camera_model.m[0][3] = -2;
  m4_view = m4_identity();

  m4_projection = m4_perspective(M_PI/3, 1000.0f/600.0f, 0.1f, 100.0f);
  m4_mvp = m4_mvp_calculate(&m4_camera_model);

  camera_pitch = 0.0f;
  camera_yaw = -M_PI/2;
  camera_sensitivity = 0.005f;

  direction_global_up = (struct v3){{{0.0f, 1.0f, 0.0f}}};

  camera_look_at(&camera_target);
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
  if (key_down(GLFW_KEY_D)) {
    camera_position = camera_go_forward();
  }
  if (key_down(GLFW_KEY_S)) {
    camera_position = camera_go_backwards();
  }
  if (key_down(GLFW_KEY_G)) {
    camera_position = camera_go_right();
  }
  if (key_down(GLFW_KEY_F)) {
    camera_position = camera_go_left();
  }
  if (key_down_single(GLFW_MOUSE_BUTTON_LEFT)) {
    info("Pressed left mouse button @ coords: (%.2f,%.2f).\n",
        cursor_x, cursor_y);
  }
}

void
globals_event_reset(void)
{
  b_mvp_recalculate = false;
  b_camera_changed = false;
  b_view_recalculate = false;
  b_cursor_changed = false;
}

void
calculate_camera_direction(void)
{
  if (!b_cursor_init) {
    cursor_prev_x = cursor_x;
    cursor_prev_y = cursor_y;
    b_cursor_init = true;
  }

  GLfloat cursor_offset_x = cursor_x - cursor_prev_x;
  GLfloat cursor_offset_y = cursor_prev_y - cursor_y;

  cursor_prev_x = cursor_x;
  cursor_prev_y = cursor_y;

  cursor_offset_x *= camera_sensitivity;
  cursor_offset_y *= camera_sensitivity;

  camera_yaw += cursor_offset_x;
  camera_pitch += cursor_offset_y;

  if (camera_pitch >= M_PI/2 ) {
    camera_pitch = M_PI/2;
  }
  if (camera_pitch <= -M_PI/2 ) {
    camera_pitch = -M_PI/2;
  }

  GLfloat dir_x = cosf(camera_yaw) * cosf(camera_pitch);
  GLfloat dir_y = -sinf(camera_pitch);
  GLfloat dir_z = sinf(camera_yaw) * cosf(camera_pitch);
  /* Update camera direction. */
  camera_direction = (struct v3) {{{dir_x, dir_y, dir_z}}};
  camera_direction = v3_normalize(&camera_direction);

  /* Re-calculate camera-right and camera-up vectors based on new direction. */
  camera_right = v3_cross(&direction_global_up, &camera_direction);
  camera_right = v3_normalize(&camera_right);
  camera_up = v3_cross(&camera_direction, &camera_right);
  camera_up = v3_normalize(&camera_up);

  b_view_recalculate = true;
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
    b_view_recalculate = true;
  }
  if (b_cursor_changed) {
    //calculate_camera_direction();
  }
  if (b_view_recalculate) {
    m4_view_calculate();
    b_mvp_recalculate = true;
  }
  if (b_mvp_recalculate) {
    m4_mvp = m4_mvp_calculate(&m4_camera_model);
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
callback_cursor_position(GLFWwindow * window, double pos_x, double pos_y)
{
  UNUSED(window);

  if (b_cursor_changed) {
    return;
  }

  cursor_x = (GLfloat)pos_x;
  cursor_y = (GLfloat)pos_y;

  b_cursor_changed = true;
}

void
callback_mouse_key(GLFWwindow * window, int button, int action, int mods)
{
  UNUSED(window); UNUSED(mods);
  event_queue_add(button, action == GLFW_PRESS ? true : false);
}
