#ifndef LUGL_H

#define LUGL_H

// Includes:
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Structs

typedef struct {
  uint8_t *data;
  size_t bytes_alloced;
  size_t bytes_added;
  size_t stride;
  unsigned int VAO, VBO;
} lu_Mesh;

// Function prototypes

// Creates and returns a pointer to a GLFWwindow
GLFWwindow *lu_create_window(const char *window_title, int width, int height, bool fullscreen);

// Creates and returns a shader program given a number of shaders, and the locations of all those shaders as variadic args.
// Fragment shaders must have the extension .frag, and .vert for vertex shaders.
// For example, create_shader_program(2, "source/shaders/vertex_shader.vert", "source/shaders/fragment_shader.frag") would compile both vertex_shader.vert and fragment_shader.frag, and link them to
// the shader program that is returned.
GLuint lu_create_shader_program(size_t num_shaders, ...);

// Defines a layout for a VAO and VBO given a number of components, their sizes, their counts, and their types
// For example, to create a layout for this struct:
// typedef struct {
//	float position[3];
//	float tex_coord[2];
// } Vertex;
//	num_components should be 2 (position & tex_coord)
//	components_sizes should be {sizeof(float), sizeof(float)};
//	component_counts should be {2, 3}
//	and component_types should be {GL_FLOAT, GL_FLOAT}
void lu_define_layout(GLuint *VAO, GLuint *VBO, size_t num_components, size_t *component_sizes, size_t *component_counts, GLenum *component_types);

// Loads a texture image into memory using stb_image.h, and sends it as a uniform to a shader program with the name in texture_uniform_name.
unsigned int lu_send_uniform_texture(char *texture_location, GLuint shader_program, char *texture_uniform_name);
// Create a mesh with the specified vertex layout
lu_Mesh lu_mesh_create(size_t num_components, size_t *component_sizes, size_t *component_counts, GLenum *component_types);
// Add a certain number of bytes to the mesh
void lu_mesh_add_bytes(lu_Mesh *mesh, void *src, size_t n_bytes);
// Free the current data stored in the mesh on the CPU, but dont delete buffers
void lu_mesh_free(lu_Mesh *mesh);
// Free all data and delete OpenGL buffers
void lu_mesh_delete(lu_Mesh *mesh);
// Render a mesh
void lu_mesh_render(lu_Mesh *mesh, GLenum render_mode);
// Send a mesh to the GPU
void lu_mesh_send(lu_Mesh *mesh);

#endif // luGL.h
