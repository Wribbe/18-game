#include <stdlib.h>
#include <stdio.h>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

int main(void)
{

    if (!glfwInit()) {
      printf("failed to initialize GLFW.\n");
      return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow * window = glfwCreateWindow(1000, 600, "awesome", NULL, NULL);

    if (!window) {
      return -1;
    }

    glfwMakeContextCurrent(window);
    if (gl3wInit()) {
      printf("failed to initialize OpenGL\n");
      return -1;
    }

    /* Initialize the library */
    if (!glfwInit()) {
        return -1;
    }

    printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION),
        glGetString(GL_SHADING_LANGUAGE_VERSION));

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
