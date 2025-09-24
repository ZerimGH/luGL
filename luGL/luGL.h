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

// Function prototypes

// Creates and returns a pointer to a GLFWwindow
GLFWwindow *lu_create_window(const char *window_title, int width, int height, bool fullscreen);

// Creates and returns a shader program given a number of shaders, and the locations of all those shaders as variadic args.
// Fragment shaders must have the extension .frag, and .vert for vertex shaders.
// For example, create_shader_program(2, "source/shaders/vertex_shader.vert", "source/shaders/fragment_shader.frag") would compile both vertex_shader.vert and fragment_shader.frag, and link them to
// the shader program that is returned.
GLuint lu_create_shader_program(int num_shaders, ...);

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
void lu_define_layout(GLuint *VAO, GLuint *VBO, int num_components, int *component_sizes, int *component_counts, GLenum *component_types);

// Loads a texture image into memory using stb_image.h, and sends it as a uniform to a shader program with the name in texture_uniform_name.
unsigned int lu_send_uniform_texture(char *texture_location, GLuint shader_program, char *texture_uniform_name);
#endif
