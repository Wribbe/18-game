#include "lib.h"

int main(void)
{

    if (!glfwInit()) {
      printf("failed to initialize GLFW.\n");
      return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow * window = glfwCreateWindow(1000, 600, "awesome", NULL, NULL);

    if (!window) {
      return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    if (gl3wInit()) {
      printf("failed to initialize OpenGL\n");
      return EXIT_FAILURE;
    }

    /* Initialize the library */
    if (!glfwInit()) {
        return EXIT_FAILURE;
    }

    printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION),
        glGetString(GL_SHADING_LANGUAGE_VERSION));

    info("Information!\n");
    error("Error!\n");

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
