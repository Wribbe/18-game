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

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        event_queue_process();

    }

    glfwTerminate();
    return EXIT_SUCCESS;
}
