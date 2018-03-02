#include "lib.h"

int main(void)
{
    GLFWwindow * window = init_window(1000, 600, "2018 -- Game");

    if (window == NULL) {
      error("Initialization failed, aborting.\n");
      return EXIT_FAILURE;
    }

    char * source_vert = read_file("src/shaders/default.vert");
    info("Read source from vertex shader: %s\n", source_vert);
    char * source_frag = read_file("src/shaders/default.frag");
    info("Read source from fragment shader: %s\n", source_frag);
    free(source_vert);
    free(source_frag);

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
    return 0;
}
