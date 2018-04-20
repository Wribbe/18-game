#include "lib.h"

int main(void)
{
  GLFWwindow * window = init_window(1000, 600, "2018 -- Game");

  if (window == NULL) {
    error("Initialization failed, aborting.\n");
    return EXIT_FAILURE;
  }

  /* Initialize sub-systems. */
  init_environment();

  program_shader_default = program_shader_create(
      "src/shaders/default.vert",
      "src/shaders/default.frag");
  if (program_shader_default == 0) {
    error("Shader program compilation failed.\n");
    return EXIT_FAILURE;
  }

  size_t num_floats = 0;
  GLfloat * floats = file_read_floats("data/basic_triangle.txt",
      &num_floats);

  struct v3 translations[] = {
    {{{0.0f, 0.0f, 0.0f}}},
    {{{1.0f, 0.0f, 0.0f}}},
    {{{0.0f, 1.0f, 0.0f}}},
    {{{-1.0f, -1.0f, 0.0f}}},
    {{{-2.0f, 0.0f, 0.0f}}},
    {{{1.0f, 1.0f, 0.0f}}},
  };

  size_t num_objects = 5;
  GLuint objects[num_objects];
  for (size_t i=0; i<num_objects; i++) {
    objects[i] = render_object_create(floats, num_floats);
    object_translate(objects[i], &translations[i]);
  }

  object_scale(objects[2], &(struct v3){{{1.0f, 0.1f, 1.0f}}});
  object_scale(objects[3], &(struct v3){{{0.1f, 1.0f, 1.0f}}});

  set_as_player(objects[4]);

  program_use(program_shader_default);

  /* Loop until the user closes the window */
  while (!glfwWindowShouldClose(window))
  {
    /* Set time-delta. */
    clock_tick();

    /* Clear color. */
    glClear(GL_COLOR_BUFFER_BIT);

    /* Poll for and process events. */
    event_queue_process();

    /* Run physics and collisions. */
    physics_tick();

    /* Draw objects. */
    draw_objects();

    /* Swap front and back buffers. */
    glfwSwapBuffers(window);
  }

  free(floats);
  glfwTerminate();
  return EXIT_SUCCESS;
}
