#include "lib.h"

/* Information output functions.
 * ----------------------------- */

void
error(const char * fmt, ...) {
  va_list ap;
  fprintf(stderr, "[!]: ");
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}

void
info(const char * fmt, ...) {
  va_list ap;
  fprintf(stdout, "[-]: ");
  va_start(ap, fmt);
  vfprintf(stdout, fmt, ap);
  va_end(ap);
}


GLFWwindow *
init_window(size_t width, size_t height, const char * title)
{
    if (!glfwInit()) {
      error("Failed to initialize GLFW.\n");
      return NULL;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow * window = glfwCreateWindow(width, height, title, NULL, NULL);

    if (window == NULL) {
      error("Failed to create window.\n");
      return NULL;
    }

    glfwMakeContextCurrent(window);
    if (gl3wInit()) {
      error("Failed to initialize OpenGL (gl3w).\n");
      return NULL;
    }

    /* Initialize the library */
    if (!glfwInit()) {
      error("Failed to initialize glfw.\n");
      return NULL;
    }

    info("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION),
        glGetString(GL_SHADING_LANGUAGE_VERSION));

    glfwSetKeyCallback(window, key_callback);
    current_window = window;
    return window;
}

char *
read_file(const char * filepath)
{
  FILE * handle = fopen(filepath, "r");
  if (handle == NULL) {
    error("Error when trying to open file %s.\n", filepath);
    return NULL;
  }
  /* Get file-size. */
  fseek(handle, 0, SEEK_END);
  size_t size_file = ftell(handle);
  rewind(handle);
  /* Allocate data. */
  char * data = malloc(sizeof(char) * (size_file + 1));
  if (data == NULL) {
    error("Could not allocate data for storing file content (%s).\n",
        filepath);
    return NULL;
  }
  /* Copy data. */
  size_t read = fread(data, size_file, 1, handle);
  if (read != 1) {
    error("Error when reading file; %s\n", filepath);
    free(data);
    return NULL;
  }
  fclose(handle);
  data[size_file] = '\0';
  return data;
}
