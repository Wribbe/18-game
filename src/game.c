#include "lib.h"

int main(void)
{
    GLFWwindow * window = init_window(1000, 600, "2018 -- Game");

    if (window == NULL) {
      error("Initialization failed, aborting.\n");
      return EXIT_FAILURE;
    }

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

    struct vao vao = buffer_create(floats, num_floats);
    glUseProgram(program_default);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        /* Draw triangle. */
        draw_arrays(GL_TRIANGLES, &vao);

        /* Poll for and process events */
        event_queue_process();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);
    }

    free(floats);
    glfwTerminate();
    return EXIT_SUCCESS;
}
