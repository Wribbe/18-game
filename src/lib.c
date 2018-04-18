#include "lib.h"
/* External definitions
 * -------------------- */

GLuint shader_program_current = 0;
GLuint shader_program_debug = 0;
GLuint shader_program_default = 0;
struct render_object render_queue[NUM_RENDER_OBJECTS] = {0};
#define FIRST_RENDER_OBJECT 1 /* 0 reserved for error. */
GLuint last_render_object = FIRST_RENDER_OBJECT;
GLuint vao_debug = 0;

/* DEBUG globals
 * ----------------- */

bool b_debug_print_bounding_squares = true;

/* Information output functions.
 * ----------------------------- */

void
error(const char * fmt, ...)
{
  va_list ap;
  fprintf(stderr, "[!]: ");
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}

void
info(const char * fmt, ...)
{
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

    glfwSetKeyCallback(window, callback_key);
    glfwSetCursorPosCallback(window, callback_cursor_position);
    current_window = window;

    /* Grab mouse pointer. */
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    /* Set mouse button callback function. */
    glfwSetMouseButtonCallback(window, callback_mouse_key);

    return window;
}

void
init_environment(void)
{
    /* Initialize camera system. */
    camera_system_init();

    /* Initialize time delta clock. */
    clock_init();

    /* Create debug shader program. */
    shader_program_debug = shader_program_create(
        "src/shaders/default.vert",
        "src/shaders/debug.frag");
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
eat_comment(char * c, char const * string_end)
  /* Eat the next comment, including newlines and return pointer to next
   * non-comment character. */
{
  if (*c == '/' && c+2 <= string_end) {
    char next = *(c+1);
    /* Multi-line comment. */
    if (next == '*') {
      /* Advance past the comment part first. */
      c += 2;
      /* Keep on going until hitting string_end of comment or string_end. */
      for (;;c++) {
        if (c >= string_end) {
          break;
        }
        if (*c == '*' && c+1 <= string_end) {
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
      /* Keep on going until string_end is hit or there is a newline. */
      for (; *c != '\n' && c <= string_end; c++){};
      /* Skip the actual newline character if string_end was not hit. */
      if (c != string_end) {
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
   *   Use expanding (realloc) array and try to convert everything to floats?
   *
   *   */
{
  char * data = file_read(filepath);

  if (data == NULL) {
    error("Could not read data that should be read as floats.\n");
    return NULL;
  }

  char * string_end = data + strlen(data);
  char * c = data;

  size_t size_increment = 1;
  size_t size_array = 1;

  GLfloat * array_floats = malloc(sizeof(GLfloat) * size_array);
  if (array_floats == NULL) {
    error("Could not allocate initial memory for float conversion.\n");
    return NULL;
  }

  char * after_float = data;
  GLfloat * p_float_current = array_floats;
  char * current_c = c;

  while (c < string_end) {
    if ((size_t)(p_float_current - array_floats) >= size_array) {
      /* Expand the float array. */
      size_t size_new = size_array + size_increment;
      GLfloat * array_expanded = realloc(array_floats, sizeof(GLfloat) *
          size_new);
      if (array_expanded == NULL) {
        error("Could not expand array used for float conversion.\n");
        free(array_floats);
        free(data);
        return NULL;
      }
      /* Re-calculate current pointer.*/
      p_float_current = array_expanded + size_array;
      /* Set expanded array as new array and update size. */
      array_floats = array_expanded;
      size_array = size_new;
    }
    /* Eat all encountered comments. */
    while ((c = eat_comment(c, string_end)) != current_c) {
      current_c = c;
    }
    *p_float_current++ = strtof(c, &after_float);
    c = after_float;
    /* Skip any 'f', ',' or white-space encountered. */
    for(; *c == 'f' || *c == ',' || isspace(*c) ; c++){};
    /* Avoid allocating new float on trailing comment in file. */
    while ((c = eat_comment(c, string_end)) != current_c) {
      current_c = c;
    }
  }
  size_t num_parsed = p_float_current - array_floats;
  /* Trim the returned array to fit number of parsed floats. */
  GLfloat * array_trimmed = realloc(array_floats, sizeof(GLfloat) * num_parsed);
  if (array_trimmed == NULL) {
    error("Error when trying to trim converted floats to size.\n");
    free(array_floats);
    free(data);
    return NULL;
  }
  /* Write size back to caller. */
  if (num_floats != NULL) {
    *num_floats = num_parsed;
  }
  free(data);
  return array_trimmed;
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

struct vao
buffer_create(GLfloat * floats, size_t num_floats)
{
  /* Set up buffer variables. */
  GLuint id_vao = 0;
  GLuint id_vbo = 0;
  /* Generate the array and buffer. */
  glGenVertexArrays(1, &id_vao);
  glGenBuffers(1, &id_vbo);
  /* Bind vertex array. */
  glBindVertexArray(id_vao);
  /* Bind buffer. */
  glBindBuffer(GL_ARRAY_BUFFER, id_vbo);
  /* Add data to buffer. */
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*num_floats, floats,
      GL_STATIC_DRAW);
  /* Set up and enable vertex attribute pointer. */
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
      (void*)0);
  glEnableVertexAttribArray(0);

  /* Unbind vertex array. */
  glBindVertexArray(0);
  /* Unbind buffer. */
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  /* Create and return struct vao */
  struct vao vao = {
    .id = id_vao,
    .num_indices = num_floats/3,
    .stride = 3,
  };
  return vao;
}

struct bound_points
make_bound_points(GLfloat * floats, size_t num_floats, struct vao * vao)
{
  if (num_floats < 3) {
    error("make_bound_points needs at least 3 floats.\n");
    return (struct bound_points){
      {{{0.0f, 0.0f, 0.0f}}},
      {{{0.0f, 0.0f, 0.0f}}},
    };
  }
  GLfloat max_x = floats[0];
  GLfloat max_y = floats[1];
  GLfloat max_z = floats[2];

  GLfloat min_x = max_x;
  GLfloat min_y = max_y;
  GLfloat min_z = max_z;

  for (size_t i=vao->stride; i<num_floats; i += vao->stride) {
    GLfloat x = floats[i];
    GLfloat y = floats[i+1];
    GLfloat z = floats[i+2];

    max_x = x > max_x ? x : max_x;
    max_y = y > max_y ? y : max_y;
    max_z = z > max_z ? z : max_z;

    min_x = x < min_x ? x : min_x;
    min_y = y < min_y ? y : min_y;
    min_z = z < min_z ? z : min_z;
  }

  return (struct bound_points) {
    {{{max_x, max_y, max_z}}},
    {{{min_x, min_y, min_z}}},
  };
}

GLuint
render_object_create(GLfloat * floats, size_t num_floats)
{
  if (last_render_object >= NUM_RENDER_OBJECTS) {
    error("Can't create render object, queue full.");
    return 0;
  }
  struct vao vao = buffer_create(floats, num_floats);
  GLuint assigned_id = last_render_object++;
  struct render_object * obj = &render_queue[assigned_id];
  obj->active = true;
  obj->render_type = GL_TRIANGLES;
  obj->vao = vao;
  obj->m4_model = m4_identity();

  obj->bound_points = make_bound_points(floats, num_floats, &vao);

  return assigned_id;
}

void
draw_arrays(GLenum type, struct vao * vao)
{
  glBindVertexArray(vao->id);
  glDrawArrays(type, 0, vao->num_indices);
  glBindVertexArray(0);
}

struct bound_square
bound_square_get(struct render_object * obj)
{
  struct v3 * top_left = &obj->bound_points.top_left;
  struct v3 * bottom_right = &obj->bound_points.bottom_right;
  return (struct bound_square){{
    *top_left,
    {{{bottom_right->x, top_left->y, top_left->z}}},
    {{{top_left->x, bottom_right->y, top_left->z}}},
    *bottom_right,
  }};
}


size_t
debug_buffers_feed(struct render_object * obj)
{
  struct bound_square bound_square = bound_square_get(obj);
  GLuint indices[] = {
    0, 1,
    0, 2,
    2, 3,
    3, 1,
  };

  if (!vao_debug) {
    /* Generate all buffers/vertex Arrays. */
    GLuint vbo_debug = 0;
    GLuint ebo_debug = 0;
    glGenVertexArrays(1, &vao_debug);
    glGenBuffers(1, &vbo_debug);
    glGenBuffers(1, &ebo_debug);
    /* Bind Array and setup buffers. */
    glBindVertexArray(vao_debug);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_debug);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bound_square), &bound_square,
        GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_debug);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
        GL_STATIC_DRAW);
    /* Set up vertex attributes. */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);
    /* Unbind everything, unbind array first. */
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  } else {
    glBindVertexArray(vao_debug);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(bound_square), 0);
    glBindVertexArray(0);
  }
  return COUNT(indices);
}

void
debug_print_bounding_squares(struct render_object * obj)
{
  size_t num_points = debug_buffers_feed(obj);
  glBindVertexArray(vao_debug);
  glDrawElements(GL_LINES, num_points, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void
draw_objects(void)
{
  for (size_t i=FIRST_RENDER_OBJECT; i<last_render_object; i++) {
    struct render_object * obj = &render_queue[i];
    m4_mvp = m4_mvp_calculate(&obj->m4_model);
    program_use(shader_program_default);
    draw_arrays(obj->render_type, &obj->vao);
    if (b_debug_print_bounding_squares) {
      program_use(shader_program_debug);
      debug_print_bounding_squares(obj);
    }
  }
}

void
program_use(GLuint id_program)
{
  shader_program_current = id_program;
  glUseProgram(id_program);
  program_bind_mat4fv(id_program, UNIFORM_NAME_MVP, &m4_mvp);
}

void
program_bind_mat4fv(GLuint id_program, const char * uniform, struct m4 * data)
{
    GLuint location = glGetUniformLocation(id_program, uniform);
    glUniformMatrix4fv(location, 1, GL_TRUE, data->m[0]);
}

void
physics_tick(void)
{
  /* Update all transformation matrices for alla render objects. */
  for (size_t i=FIRST_RENDER_OBJECT; i<last_render_object; i++) {
    struct render_object * obj = &render_queue[i];
    m4_mvp_calculate(&obj->m4_model);
  }
}
