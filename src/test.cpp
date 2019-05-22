#include "linmath.h"
#include <GLFW/glfw3.h>
#include <OpenGL/gl.h>
#include <array>
#include <iostream>

namespace
{
const char* vt_sh = "#version 110\n"
                    "uniform mat4 MVP;\n"
                    "attribute vec3 vCol;\n"
                    "attribute vec3 vPos;\n"
                    "varying vec3 color;\n"
                    "void main() {\n"
                    "    gl_Position = MVP * vec4(vPos, 1.0);\n"
                    "    color = vCol;\n"
                    "}\n";
const char* fg_sh = "#version 110\n"
                    "varying vec3 color;\n"
                    "void main() {\n"
                    "    gl_FragColor = vec4(color, 1.0);\n"
                    "}\n";

GLuint vertex_shader, fragment_shader, program;

GLuint vertex_buffer[1];
GLint  MVP, vpos, vcol;

struct Vertex
{
  float x, y, z;
  float r, g, b;
};
std::array<Vertex, 4> vertices = {{
    {-1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f},
    {1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f},
    {1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f},
    {-1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f},
}};
} // namespace

void
testInit()
{
  // シェーダ生成
  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vt_sh, nullptr);
  glCompileShader(vertex_shader);
  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fg_sh, nullptr);
  glCompileShader(fragment_shader);
  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);
  MVP  = glGetUniformLocation(program, "MVP");
  vpos = glGetAttribLocation(program, "vPos");
  vcol = glGetAttribLocation(program, "vCol");

  // 頂点生成
  glGenBuffers(1, vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(),
               vertices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void
testSetup()
{
}

void
testRender(GLFWwindow* window)
{
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  float ratio = width / (float)height;

  mat4x4 m, p, mvp;
  mat4x4_identity(m);
  mat4x4_rotate_Z(m, m, (float)glfwGetTime());
  mat4x4_translate_in_place(m, 0.0f, 0.0f, -0.5f);
  // mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
  mat4x4_frustum(p, -ratio, ratio, -1.0f, 1.0f, 0.1f, 1000.0f);
  mat4x4_mul(mvp, p, m);

  glUseProgram(program);
  glUniformMatrix4fv(MVP, 1, GL_FALSE, (const GLfloat*)mvp);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[0]);
  glEnableVertexAttribArray(vpos);
  glVertexAttribPointer(vpos, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
  glEnableVertexAttribArray(vcol);
  glVertexAttribPointer(vcol, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        &((Vertex*)0)->r);
  glDrawArrays(GL_QUADS, 0, 4);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDisableVertexAttribArray(vpos);
  glDisableVertexAttribArray(vcol);
}
