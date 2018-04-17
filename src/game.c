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

  GLuint program_default = shader_program_create(
      "src/shaders/default.vert",
      "src/shaders/default.frag");
  if (program_default == 0) {
    error("Shader program compilation failed.\n");
    return EXIT_FAILURE;
  }

  size_t num_floats = 0;
  GLfloat * floats = file_read_floats("data/basic_triangle.txt",
      &num_floats);

  render_object_create(floats, num_floats);
  GLuint id2 = render_object_create(floats, num_floats);
  object_translate(id2, &(struct v3){{{0.3f, 0.3f, 0.3f}}});
  program_use(program_default);

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
