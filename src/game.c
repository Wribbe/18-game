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

    struct v3 camera_position = {{{0.0f, 0.0f, 3.0f}}};
    struct v3 camera_target = {{{0.0f, 0.0f, 0.0f}}};
    struct v3 direction_up = {{{0.0f, 1.0f, 0.0f}}};

    struct v3 camera_direction = v3_sub(&camera_position, &camera_target);
    camera_direction = v3_normalize(&camera_direction);

    struct v3 camera_right = v3_cross(&direction_up, &camera_direction);
    camera_right = v3_normalize(&camera_right);

    struct m4 model = {{
      {1.0f, 0.0f, 0.0f, 0.0f},
      {0.0f, 1.0f, 0.0f, 0.0f},
      {0.0f, 0.0f, 1.0f, 0.0f},
      {0.0f, 0.0f, 0.0f, 1.0f},
    }};
    struct m4 view = {{
      {1.0f, 0.0f, 0.0f, 0.0f},
      {0.0f, 1.0f, 0.0f, 0.0f},
      {0.0f, 0.0f, 1.0f, 0.0f},
      {0.0f, 0.0f, 0.0f, 1.0f},
    }};
    struct m4 projection = {{
      {1.0f, 0.0f, 0.0f, 0.0f},
      {0.0f, 1.0f, 0.0f, 0.0f},
      {0.0f, 0.0f, 1.0f, 0.0f},
      {0.0f, 0.0f, 0.0f, 1.0f},
    }};

    GLuint location_model = glGetUniformLocation(program_default,
        "model");
    GLuint location_view = glGetUniformLocation(program_default,
        "view");
    GLuint location_projection = glGetUniformLocation(program_default,
        "projection");

    glUniformMatrix4fv(location_model, 1, GL_TRUE, model.m[0]);
    glUniformMatrix4fv(location_view, 1, GL_TRUE, view.m[0]);
    glUniformMatrix4fv(location_projection, 1, GL_TRUE, projection.m[0]);

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
