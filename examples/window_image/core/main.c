#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#include "luGL.h"

struct Vertex {
  float pos[2];
  float tex_coord[2];
};

// Vertex information for opengl
size_t vertex_num_components = 2;                                 // 2 components: pos and texcoord
size_t vertex_component_counts[] = {2, 2};                        // The number of parts to each component, pos is a length 2 array, so it has 2 parts, texcoord is also a length 2 array, so 2 parts.
size_t vertex_component_sizes[] = {sizeof(float), sizeof(float)}; // The size of each individual part of a component, each are float arrays, so it is the size of a float.
GLenum vertex_component_types[] = {GL_FLOAT, GL_FLOAT};        // The type that OpenGL should interpret the components as.

int main() {
  // Create a window to draw to
  GLFWwindow *window = lu_create_window("window!", 600, 600, false);

  // Define quad vertices
  struct Vertex quad[] = {
      // Position	// Texcoord
      {{-1, 1}, {0, 1}}, {{-1, -1}, {0, 0}}, {{1, -1}, {1, 0}}, {{1, 1}, {1, 1}}, {{-1, 1}, {0, 1}}, {{1, -1}, {1, 0}},
  };

  // Create a mesh to store the quad
  lu_Mesh mesh = lu_mesh_create(vertex_num_components, vertex_component_sizes, vertex_component_counts, vertex_component_types);
  lu_mesh_add_bytes(&mesh, &quad, sizeof(quad));
  lu_mesh_send(&mesh);
  // Can free mesh on the CPU now
  lu_mesh_free(&mesh);

  // Load shaders
  GLuint shader_program = lu_create_shader_program(2, "shaders/vert.vert", "shaders/frag.frag");

  // Send texture image to shader program as uniform
  unsigned int texture = lu_send_uniform_texture("textures/hyrax.jpg", shader_program, "tex");

  // Keep window running
  while (!glfwWindowShouldClose(window)) {
    // Draw to the window and swap buffers
    glClearColor(0.18f, 0.14f, 0.17f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    // Draw quad
    glBindTexture(GL_TEXTURE_2D, texture);
    glUseProgram(shader_program);
    lu_mesh_render(&mesh, GL_TRIANGLES);
    glfwSwapBuffers(window);
    // Poll for events
    glfwPollEvents();
  }

  // Clean up when the window is closed
  glfwDestroyWindow(window);
  glDeleteProgram(shader_program);
  lu_mesh_delete(&mesh);
  return 0;
}
