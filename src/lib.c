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
file_read(const char * filepath)
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

char *
eat_comment(char * c, char const * end)
  /* Eat the next comment, including newlines and return pointer to next
   * non-comment character. */
{
  if (*c == '/' && c+2 <= end) {
    char next = *(c+1);
    /* Multi-line comment. */
    if (next == '*') {
      /* Advance past the comment part first. */
      c += 2;
      /* Keep on going until hitting end of comment or end. */
      for (;;c++) {
        if (c >= end) {
          break;
        }
        if (*c == '*' && c+1 <= end) {
          next = *(c+1);
          if (next == '/') {
            /* Skip over the next char. */
            c += 2;
            /* If the next char is a newline, skip that too. */
            if (*c == '\n') {
              c++;
            }
            break;
          }
        }
      }
    /* Single line comment. */
    } else if (next == '/') {
      /* Advance past comment part as previously. */
      c += 2;
      /* Keep on going until end is hit or there is a newline. */
      for (; *c != '\n' && c <= end; c++){};
      /* Skip the actual newline character if end was not hit. */
      if (c != end) {
        c++;
      }
    }
  }
  return c;
}

GLfloat *
file_read_floats(const char * filepath, size_t * num_floats)
  /* Read file contents, and return a array of floats.
   *
   *   Note: Could probably _try_ to do something fancy here and count the
   *   number of commas etc. Will probably fail. Also want to ignore comments.
   *   Use expanding (calloc) array and try to convert everything to floats?
   *
   *   */
{
  char * data = file_read(filepath);

  if (data == NULL) {
    error("Could not read data that should be read as floats.\n");
    return NULL;
  }

  char * end = data + strlen(data);

  char * c = data;
  for (; c <= end; c++) {
    c = eat_comment(c, end);
    printf("%c", *c);
  }
  free(data);
  return 0;
}

GLuint
shader_program_create(const char * path_vertex, const char * path_fragment)
{

  /* Set up buffer for reporting any shader compilation and program linking
   * errors.
   *
   *   Note: This needs to be before any 'goto' statements because of the
   *   dynamic allocation of buffer_shader_error, as per the C99 standard.
   *
   * */
  int status = 0;
  size_t size_buffer_shader_error = 512;
  char buffer_shader_error[size_buffer_shader_error];

  /* Set up shader primitives for use later. */
  GLuint shader_vertex = glCreateShader(GL_VERTEX_SHADER);
  GLuint shader_fragment = glCreateShader(GL_FRAGMENT_SHADER);
  GLuint program_shader = glCreateProgram();

  /* Read file sources. */
  char * source_vertex = file_read(path_vertex);
  char * source_fragment = file_read(path_fragment);

  if (source_vertex == NULL || source_fragment == NULL) {
    error("Failed to read source for shader(s).\n");
    goto error;
  }

  /* Compile and check vertex shader. */
  glShaderSource(shader_vertex, 1, (const char **)&source_vertex, NULL);
  glCompileShader(shader_vertex);
  glGetShaderiv(shader_vertex, GL_COMPILE_STATUS, &status);
  if (status == 0) {
    glGetShaderInfoLog(shader_fragment, size_buffer_shader_error, NULL,
        buffer_shader_error);
    error("Compilation of vertex shader failed with: \n%s\nSource:\n%s\n",
        buffer_shader_error, source_vertex);
    goto error;
  }

  /* Compile and check fragment shader. */
  glShaderSource(shader_fragment, 1, (const char **)&source_fragment, NULL);
  glCompileShader(shader_fragment);
  glGetShaderiv(shader_fragment, GL_COMPILE_STATUS, &status);
  if (status == 0) {
    glGetShaderInfoLog(shader_fragment, size_buffer_shader_error, NULL,
        buffer_shader_error);
    error("Compilation of fragment shader failed with: \n%s\nSource:\n%s\n",
        buffer_shader_error, source_fragment);
    goto error;
  }

  /* Link the shaders to the shader program. */
  glAttachShader(program_shader, shader_vertex);
  glAttachShader(program_shader, shader_fragment);
  glLinkProgram(program_shader);
  /* Check the linking. */
  if (status == 0) {
    glGetProgramInfoLog(program_shader, size_buffer_shader_error, NULL,
        buffer_shader_error);
    error("Linking of shader program failed with: %s\n", buffer_shader_error);
    goto error;
  }

  /* Delete shaders and free memory allocated for sources. */
  glDeleteShader(shader_vertex);
  glDeleteShader(shader_fragment);
  free(source_vertex);
  free(source_fragment);

  return program_shader;

error:

  if (source_vertex != NULL) {
    free(source_vertex);
  }
  if (source_fragment != NULL) {
    free(source_fragment);
  }
  glDeleteShader(shader_vertex);
  glDeleteShader(shader_fragment);
  glDeleteProgram(program_shader);
  return 0;
}
