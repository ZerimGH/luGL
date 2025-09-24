#include "luGL.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stolen/stb_image.h"

#include <stdlib.h>

GLFWwindow *lu_create_window(const char *window_title, int width, int height, bool fullscreen) {
  // Initialise GLFW
  if (glfwInit() != GLFW_TRUE) {
    fprintf(stderr, "(lu_create_window): Error intitialising GLFW.\n");
    return NULL;
  }
  // Create the window
  GLFWwindow *window;
  if (!fullscreen)
    window = glfwCreateWindow(width, height, window_title, NULL, NULL);
  else
    window = glfwCreateWindow(width, height, window_title, glfwGetPrimaryMonitor(), NULL);
  if (window == NULL) {
    fprintf(stderr, "(lu_create_window): Error creating window with glfwCreateWindow().\n");
    return NULL;
  }
  glfwMakeContextCurrent(window); // Set the context as current

  // Initialise GLEW
  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "(lu_create_window): Error initialising GLEW.\n");
    return NULL;
  }

  return window;
}

static char *lu_read_file(const char *file_name, size_t *file_len) {
  FILE *ptr = fopen(file_name, "rb"); // Open the file for reading
  if (ptr == NULL) {
    fprintf(stderr, "(lu_read_file): Could not open file %s, returning NULL.\n", file_name);
    return NULL;
  }
  fseek(ptr, 0, SEEK_END);
  (*file_len) = ftell(ptr);
  if ((*file_len) == 0) {
    fclose(ptr);
    fprintf(stderr, "(lu_read_file): Could not read file %s, file has 0 length.", file_name);
    return NULL;
  }

  // Allocate output
  char *out = malloc(sizeof(char) * (*file_len + 1));

  // Read the file
  fseek(ptr, 0, SEEK_SET);
  if (fread((void *)out, 1, (*file_len), ptr) != (*file_len)) {
    fprintf(stderr, "(lu_read_file): Could not read file %s, fread() failed.\n", file_name);
    fclose(ptr);
    return NULL;
  }
  // Null terminate
  out[*file_len] = '\0';
  // Close file
  fclose(ptr);
  return out;
}

static GLuint lu_compile_shader(const char *shader_file_location) {
  // Detect the shader type based on file extension
  size_t name_len = strlen(shader_file_location);
  const char *ext = shader_file_location + name_len - 5;

  GLenum shader_type;
  if (strcmp(ext, ".vert") == 0)
    shader_type = GL_VERTEX_SHADER;
  else if (strcmp(ext, ".frag") == 0)
    shader_type = GL_FRAGMENT_SHADER;
  else {
    fprintf(stderr, "(lu_compile_shader): Unsupported extension in %s (use .vert or .frag)\n", shader_file_location);
    return 0;
  }

  // Read the shader file's contents into a string
  size_t src_len;
  char *source = lu_read_file(shader_file_location, &src_len);

  if (!source) {
    fprintf(stderr, "(lu_compile_shader): Failed to read %s\n", shader_file_location);
    return 0;
  }

  // Create and compile the shader
  GLuint shader = glCreateShader(shader_type);
  const GLchar *src = source;
  GLint len = (GLint)src_len;
  glShaderSource(shader, 1, &src, &len);
  free(source);

  glCompileShader(shader);

  // Check for compilation errors
  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char log[1024];
    glGetShaderInfoLog(shader, sizeof(log), NULL, log);
    fprintf(stderr, "(lu_compile_shader): Compilation failed for %s:\n%s", shader_file_location, log);
    glDeleteShader(shader);
    return 0;
  }

  return shader;
}

GLuint lu_create_shader_program(size_t num_shaders, ...) {
  va_list args;
  va_start(args, num_shaders);

  GLuint *shaders = malloc(sizeof(unsigned int) * num_shaders);
  for (int i = 0; i < num_shaders; i++) {
    // Create and compile each shader
    char *shader_file_name = va_arg(args, char *);
    GLuint shader = lu_compile_shader(shader_file_name);
    // Check for compilation error
    if (shader == 0) {
      fprintf(stderr, "(lu_create_shader_program): Couldn't create shader program, compilation of shader %s failed.\n", shader_file_name);
      // Delete all other shaders on failure to compile
      for (int j = 0; j < i; j++) {
        glDeleteShader(shaders[j]);
      }
      free(shaders);
      return 0;
    }

    shaders[i] = shader;
  }
  va_end(args);

  // Create the program
  GLuint shader_program = glCreateProgram();
  for (int i = 0; i < num_shaders; i++) {
    glAttachShader(shader_program, shaders[i]);
  }

  glLinkProgram(shader_program);
  // Delete all shaders now, stack overflow says i can do it lol
  for (int i = 0; i < num_shaders; i++) {
    glDeleteShader(shaders[i]);
  }
  free(shaders);

  // Check for linking errors
  GLint success;
  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  if (!success) {
    char log[1024];
    glGetProgramInfoLog(shader_program, sizeof(log), NULL, log);
    fprintf(stderr, "(lu_create_shader_program): Shader linking failed:\n%s", log);
    glDeleteProgram(shader_program);
    return 0;
  }

  return shader_program;
}

void lu_define_layout(GLuint *VAO, GLuint *VBO, size_t num_components, size_t *component_sizes, size_t *component_counts, GLenum *component_types) {
  // Clear whatever might exist in the VAO and VBO
  glDeleteVertexArrays(1, VBO);
  glGenBuffers(1, VBO);
  glBindBuffer(GL_ARRAY_BUFFER, *VBO);

  glDeleteVertexArrays(1, VAO);
  glGenVertexArrays(1, VAO);
  glBindVertexArray(*VAO);

  // Calculate stride
  int stride = 0;
  for (int i = 0; i < num_components; i++) {
    stride += component_sizes[i] * component_counts[i];
  }
  // Attrib pointer to each component
  int offset = 0;
  for (int i = 0; i < num_components; i++) {
    glVertexAttribPointer(i, component_counts[i], component_types[i], GL_FALSE, stride, (GLvoid *)(intptr_t)offset);
    glEnableVertexAttribArray(i);
    offset += component_sizes[i] * component_counts[i];
  }
}

unsigned int lu_send_uniform_texture(char *texture_location, GLuint shader_program, char *texture_uniform_name) {
  unsigned int texture = 0;

  int image_width, image_height, comp; // No idea what comp is
  stbi_set_flip_vertically_on_load(1);
  char *image = (char *)stbi_load(texture_location, &image_width, &image_height, &comp, STBI_rgb_alpha);

  if (image == NULL) {
    fprintf(stderr, "(lu_send_uniform_texture): Error loading image file, stbi_load returned NULL.\n");
    return 0;
  }
  // Create and bind the texture
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  // Set parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // Not quite sure what this does
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

  // Send the texture as a uniform
  glUseProgram(shader_program);
  glUniform1i(glGetUniformLocation(shader_program, texture_uniform_name), 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);

  // Free image
  stbi_image_free(image);

  // Unbind texture
  glBindTexture(GL_TEXTURE_2D, 0);

  return texture;
}

lu_Mesh lu_mesh_create(size_t num_components, size_t *component_sizes, size_t *component_counts, GLenum *component_types) {
  lu_Mesh mesh = {0};
  mesh.data = NULL;
  mesh.bytes_alloced = 0;
  mesh.bytes_added = 0;
  mesh.stride = 0;
  for (int i = 0; i < num_components; i++) {
    mesh.stride += component_sizes[i] * component_counts[i];
  }

  lu_define_layout(&mesh.VAO, &mesh.VBO, num_components, component_sizes, component_counts, component_types);
  return mesh;
}

void lu_mesh_add_bytes(lu_Mesh *mesh, void *src, size_t n_bytes) {
  if (n_bytes == 0) return;
  if (!src) return;
  if (!mesh) return;

  if (!mesh->data) {
    mesh->bytes_alloced = 64;
    mesh->data = malloc(sizeof(uint8_t) * mesh->bytes_alloced);
    mesh->bytes_added = 0;
  }

  if (mesh->bytes_added + n_bytes > mesh->bytes_alloced) {
    while (mesh->bytes_added + n_bytes > mesh->bytes_alloced) {
      mesh->bytes_alloced *= 2;
    }
    mesh->data = realloc(mesh->data, mesh->bytes_alloced * sizeof(uint8_t));
  }

  memcpy(mesh->data + mesh->bytes_added, src, n_bytes);
  mesh->bytes_added += n_bytes;
}

void lu_mesh_free(lu_Mesh *mesh) {
  if (!mesh) return;
  if (mesh->data) free(mesh->data);
  mesh->data = NULL;
  mesh->bytes_alloced = 0;
  // dont reset mesh->bytes_added, lu_mesh_add_bytes will do that for us
}

void lu_mesh_delete(lu_Mesh *mesh) {
  if (!mesh) return;
  lu_mesh_free(mesh);
  glDeleteVertexArrays(1, &(mesh->VAO));
  glDeleteBuffers(1, &(mesh->VBO));
}

static void lu_mesh_bind(lu_Mesh *mesh) {
  glBindVertexArray(mesh->VAO);
  if (!mesh) return;
  glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
}

static void lu_mesh_unbind() {
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void lu_mesh_send(lu_Mesh *mesh) {
  if (!mesh) return;
  if (!mesh->data) return;
  lu_mesh_bind(mesh);
  glBufferData(GL_ARRAY_BUFFER, mesh->bytes_added, mesh->data, GL_STATIC_DRAW);
  lu_mesh_unbind();
}

void lu_mesh_render(lu_Mesh *mesh, GLenum render_mode) {
  lu_mesh_bind(mesh);
  glDrawArrays(render_mode, 0, mesh->bytes_added / mesh->stride);
  lu_mesh_unbind();
}
