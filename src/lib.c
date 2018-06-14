#include "lib.h"
/* External definitions
 * -------------------- */

GLuint program_shader_current = 0;
GLuint program_shader_debug = 0;
GLuint program_shader_default = 0;
struct render_object render_queue[NUM_RENDER_OBJECTS] = {0};
#define FIRST_RENDER_OBJECT 1 /* 0 reserved for error. */
GLuint last_render_object = FIRST_RENDER_OBJECT;
GLuint vao_debug = 0;
GLuint id_object_player = 0;

struct v3 COLOR_RED = {{{1.0f, 0.0f, 0.0f}}};
struct v3 COLOR_GREEN = {{{0.0f, 1.0f, 0.0f}}};

/* DEBUG globals
 * ----------------- */

bool b_debug_draw_bounding_squares = true;

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
  program_shader_debug = program_shader_create(
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
    }
    /* Single line comment. */
    else if (next == '/') {
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
program_shader_create(const char * path_vertex, const char * path_fragment)
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

struct v3
make_point_middle(struct render_object * obj)
{
  struct v3 top_left = obj->bound_points.top_left;
  struct v3 point_diff = v3_sub(&top_left, &obj->bound_points.bottom_right);
  point_diff = v3_abs(&point_diff);
  GLfloat mid_width = 0.5 * point_diff.x;
  GLfloat mid_height = 0.5 * point_diff.y;
  GLfloat mid_depth = 0.5 * point_diff.z;
  return (struct v3){{{
    top_left.x + mid_width,
    top_left.y - mid_height,
    top_left.z - mid_depth,
  }}};
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
    {{{min_x, max_y, max_z}}},
    {{{max_x, min_y, min_z}}},
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
  struct render_object * obj = get_render_object(assigned_id);
  obj->active = true;
  obj->render_type = GL_TRIANGLES;
  obj->vao = vao;
  obj->m4_model = m4_identity();
  obj->m4_model_delta = m4_identity();
  obj->m4_model_applied = m4_identity();

  obj->bound_points = make_bound_points(floats, num_floats, &vao);
  obj->point_middle = make_point_middle(obj);

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
get_bound_square(struct render_object * obj)
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

#define V3_OUTSIDE(v,i,comp,a,b) (v->i comp a->i && v->i comp b->i)

bool
v3_between(struct v3 * check, struct v3 * a, struct v3 * b)
{
  if (V3_OUTSIDE(check,x,<,a,b) || V3_OUTSIDE(check,x,>,a,b)) {
    return false;
  }
  if (V3_OUTSIDE(check,y,<,a,b) || V3_OUTSIDE(check,y,>,a,b)) {
    return false;
  }
  return true;
}

void
objects_set_colliding(GLuint id1, GLuint id2, bool value)
{
  get_render_object(id1)->state.colliding = value;
  get_render_object(id2)->state.colliding = value;
}

struct v3
model_apply_x(struct v3 * p, struct render_object * obj)
{
  struct v3 ret = {0};
  struct v3 part = {0};
  v3_copy(&ret, p);
  v3_copy(&part, p);
  ret = m4_mul_v3(&obj->m4_model, &ret);
  part = m4_mul_v3(&obj->m4_model_applied, &part);
  ret.x = part.x;
  return ret;
}

struct v3
model_apply_y(struct v3 * p, struct render_object * obj)
{
  struct v3 ret = {0};
  struct v3 part = {0};
  v3_copy(&ret, p);
  v3_copy(&part, p);
  ret = m4_mul_v3(&obj->m4_model, &ret);
  part = m4_mul_v3(&obj->m4_model_applied, &part);
  ret.y = part.y;
  return ret;
}

struct v3
get_penetration_vector(struct info_collision * info_collision,
    bool invert, struct v3 * check, struct v3 * a, struct v3 * b)
  /* Return penetration vector between the check point and either of
   * points a and b, depending on which one is closest to previous point.
   *
   * Inverts returned pointer if the checked object is anything other than
   * player object.
   */
{
  struct v3 check_a = v3_sub(a, check);
  struct v3 check_b = v3_sub(b, check);

  struct v3 ret = {{{0}}};

  if (info_collision->x) {
    if (fabsf(check_a.x) < fabsf(check_b.x)) {
      v3_copy(&ret, &check_a);
    } else {
      v3_copy(&ret, &check_b);
    }
  } else if (info_collision->y) {
    if (fabsf(check_a.y) < fabsf(check_b.y)) {
      v3_copy(&ret, &check_a);
    } else {
      v3_copy(&ret, &check_b);
    }
  } else {
    ret = v3_add(&check_a, &check_b);
  }

  if (invert) {
    ret = v3_invert(&ret);
  }

  return ret;
}


bool
object_intersects_player(GLuint id, struct info_collision * info_collision)
{
  if (INVALID_OBJECT_ID(id)) {
    error("object_intersects_player got invalid id: %u\n", id);
    return false;
  }

  struct render_object * obj_player = get_render_object(id_object_player);
  struct render_object * obj = get_render_object(id);

  struct bound_square bounds_obj = obj->bound_square_model;
  struct bound_square bounds_player = obj_player->bound_square_model;

  bool collision = false;

  struct render_object * check_object = NULL;
  struct v3 * raw_point_check = NULL;
  struct v3 * point_check = NULL;
  struct v3 * point_a = NULL;
  struct v3 * point_b = NULL;
  bool invert_penetration_vector = false;

  for (size_t i=0; i<COUNT(bounds_player.points); i++) {
    for (size_t j=1; j<COUNT(bounds_obj.points); j++) {
      if (v3_between(&bounds_player.points[i], &bounds_obj.points[j-1],
            &bounds_obj.points[j]))
      {
        raw_point_check = &obj_player->bound_square.points[i];
        point_check = &bounds_player.points[i];
        point_a = &bounds_obj.points[j-1];
        point_b = &bounds_obj.points[j];
        check_object = obj_player;
        collision = true;
      } else if (v3_between(&bounds_obj.points[i], &bounds_player.points[j-1],
            &bounds_player.points[j]))
      {
        raw_point_check = &obj->bound_square.points[i];
        point_check = &bounds_obj.points[i];
        point_a = &bounds_player.points[j-1];
        point_b = &bounds_player.points[j];
        check_object = obj_player;
        collision = true;
        invert_penetration_vector = true;
      }
      if (collision) {
        objects_set_colliding(id_object_player, id, true);
        info_collision->x = true;
        info_collision->y = true;
        struct v3 check_applied_x = model_apply_x(raw_point_check,
            check_object);
        struct v3 check_applied_y = model_apply_y(raw_point_check,
            check_object);
        if (!v3_between(&check_applied_x, point_a, point_b)) {
          info_collision->x = false;
        } else if (!v3_between(&check_applied_y, point_a, point_b)) {
          info_collision->y = false;
        }
        info_collision->penetration_vector =
          get_penetration_vector(info_collision, invert_penetration_vector,
              point_check, point_a, point_b);
        return true;
      }
    }
  }
  return false;
}

GLuint vbo_debug = 0;
GLuint ebo_debug = 0;

size_t
debug_buffers_feed(struct render_object * obj)
{
  struct bound_square bound_square = get_bound_square(obj);

  GLuint indices[] = {
    0, 1,
    0, 2,
    2, 3,
    3, 1,
  };

  if (!vao_debug) {
    /* Generate all buffers/vertex Arrays. */
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
    /* Unbind everything, order of ELEMENT matters, but not ARRAY! */
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  } else {
    glBindBuffer(GL_ARRAY_BUFFER, vbo_debug);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(bound_square), &bound_square);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
  return COUNT(indices);
}

void
debug_draw_bounding_squares(struct render_object * obj)
{
  size_t num_points = debug_buffers_feed(obj);
  glBindVertexArray(vao_debug);
  glDrawElements(GL_LINES, num_points, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void
object_state_clear(GLuint id)
{
  struct render_object * obj = get_render_object(id);
  obj->state.colliding = false;
}

bool
object_state(GLuint id, enum obj_state state)
{
  struct render_object * obj = get_render_object(id);
  switch (state) {
    case OBJ_STATE_COLLISION:
      return obj->state.colliding;
      break;
    default:
      return false;
      break;
  }
}


void
draw_object(GLuint id)
{
  struct render_object * obj = get_render_object(id);
  m4_mvp = m4_mvp_calculate(&obj->m4_model);
  program_use(program_shader_default);
  draw_arrays(obj->render_type, &obj->vao);
  if (b_debug_draw_bounding_squares) {
    program_use(program_shader_debug);
    if (object_state(id, OBJ_STATE_COLLISION)) {
      debug_program_set_border_color(&COLOR_RED);
    } else {
      debug_program_set_border_color(&COLOR_GREEN);
    }
    debug_draw_bounding_squares(obj);
  }
  object_state_clear(id);
}

void
draw_objects(void)
{
  for (GLuint i=FIRST_RENDER_OBJECT; i<last_render_object; i++) {
    if (i != id_object_player) {
      draw_object(i);
    }
  }
  draw_object(id_object_player);
}

void
debug_program_set_border_color(struct v3 * color)
{
  program_bind_3fv(program_shader_debug, UNIFORM_NAME_COLOR_BORDER, color);
}

void
program_use(GLuint id_program)
{
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
program_bind_3fv(GLuint id_program, const char * uniform, struct v3 * data)
{
  GLuint location = glGetUniformLocation(id_program, uniform);
  glUniform3fv(location, 1, data->a);
}

struct render_object *
get_render_object(GLuint id)
{
  if (INVALID_OBJECT_ID(id)) {
    error("get_render_object received invalid object id: %u\n", id);
    return NULL;
  }
  return &render_queue[id];
}

void
object_apply_delta(GLuint id)
{
  struct render_object * obj = get_render_object(id);

  obj->bound_square = get_bound_square(obj);
  object_translate(id, &obj->state.force);

  struct bound_square * bounds_model = &obj->bound_square_model;
  *bounds_model = obj->bound_square;

  obj->m4_model_applied = m4_mul(&obj->m4_model_delta, &obj->m4_model);

  for (size_t i=0; i<COUNT(bounds_model->points); i++) {
    bounds_model->points[i] = m4_mul_v3(&obj->m4_model_applied,
        &bounds_model->points[i]);
  }
  obj->point_middle_model = m4_mul_v3(&obj->m4_model_applied,
      &obj->point_middle);
}

void
object_advance(GLuint id)
{
  struct render_object * obj = get_render_object(id);
  m4_copy(&obj->m4_model, &obj->m4_model_applied);
}

void
objects_apply_delta(void)
{
  for (size_t i=FIRST_RENDER_OBJECT; i<last_render_object; i++) {
    object_apply_delta(i);
  }
}

void
objects_advance(void)
{
  for (size_t i=FIRST_RENDER_OBJECT; i<last_render_object; i++) {
    object_advance(i);
  }
}

void
object_repel(GLuint id_actor, GLuint id_target,
    struct info_collision * info_collision)
{
  struct render_object * actor = get_render_object(id_actor);
  struct render_object * target = get_render_object(id_target);

  struct v3 target_vector = v3_sub(&target->point_middle_model,
      &actor->point_middle_model);

  struct v3 right = {{{1.0f, 0.0f, 0.0f}}};
  GLfloat target_angle = v3_angle(&target_vector, &right);
  if (info_collision->x && info_collision->y) {
    printf("Came on diagonal!\n");
  } else if (info_collision->x) {
    printf("Came from side!\n");
    printf("Should move: %f in x-axis\n", info_collision->penetration_vector.x);
  } else {
    printf("Came from top/bottom!\n");
    printf("Should move: %f in y-axis\n", info_collision->penetration_vector.y);
  }
//  if (target_angle > -M_PI/2 && target_angle < M_PI/2) {
//    v3_print(info_collision);
//    target->state.force.x = 0;
//    object_add_force(id_target, &(struct v3){{{info_collision->x, 0.0f,
//        0.0f}}});
//    object_apply_delta(id_target);
//  }
}

void
objects_resolve_collisions(void)
{
  for (size_t i=FIRST_RENDER_OBJECT; i<last_render_object; i++) {
    struct info_collision info_collision = {0};
    if (i != id_object_player && object_intersects_player(i, &info_collision)) {
      object_repel(i, id_object_player, &info_collision);
    }
  }
}

void
objects_reset_forces(void)
{
  for (size_t i=FIRST_RENDER_OBJECT; i<last_render_object; i++) {
    struct render_object * obj = get_render_object(i);
    obj->state.force = (struct v3){{{0.0f, 0.0f, 0.0f}}};
    obj->m4_model_delta = m4_identity();
  }
}

void
physics_tick(void)
{
  objects_apply_delta();
  objects_resolve_collisions();
  objects_advance();
  objects_reset_forces();
}

void
set_as_player(GLuint object_id)
{
  if (INVALID_OBJECT_ID(object_id)) {
    error("set_as_player received invalid id: %u\n", object_id);
    return;
  }
  id_object_player = object_id;
}
