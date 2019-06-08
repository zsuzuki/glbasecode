#include "gl.h"
// ↑windowsでのdefineの都合上、一番先頭に置く
#include "linmath.h"
#include "primitive2d.h"
#include <array>
#include <cmath>
#include <iostream>

namespace Primitive2D
{
//
namespace
{
const char* vt_sh = "#version 110\n"
                    "uniform mat4 MVP;\n"
                    "attribute vec3 vCol;\n"
                    "attribute vec2 vPos;\n"
                    "varying vec3 color;\n"
                    "void main() {\n"
                    "    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
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

} // namespace

//
//
//
void
initialize()
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
}

//
//
//
void
terminate()
{
  glDeleteBuffers(1, vertex_buffer);
  glDeleteProgram(program);
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);
}

//
//
//
void
setup(GLFWwindow* window)
{
  auto ws    = Graphics::getWindowSize();
  auto ratio = ws.width / ws.height;

  mat4x4 mvp;
  mat4x4_ortho(mvp, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);

  glUseProgram(program);
  glUniformMatrix4fv(MVP, 1, GL_FALSE, (const GLfloat*)mvp);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[0]);
  glEnableVertexAttribArray(vpos);
  glVertexAttribPointer(vpos, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        &((Vertex*)0)->x);
  glEnableVertexAttribArray(vcol);
  glVertexAttribPointer(vcol, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        &((Vertex*)0)->r);

  glDisable(GL_BLEND);
}

//
//
//
void
cleanup()
{
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDisableVertexAttribArray(vpos);
  glDisableVertexAttribArray(vcol);
}

//
// primitive draw
//
namespace
{
void
draw(const VertexList& vlist, GLenum p)
{
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vlist.size(), vlist.data(),
               GL_STATIC_DRAW);
  glDrawArrays(p, 0, vlist.size());
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}
} // namespace

void
drawLine(const VertexList& vlist, float w)
{
  glLineWidth(w);
  draw(vlist, GL_LINE_STRIP);
}

void
drawQuads(const VertexList& vlist)
{
  draw(vlist, GL_QUADS);
}

void
drawTriangles(const VertexList& vlist)
{
  draw(vlist, GL_TRIANGLES);
}

void
drawCircle(const Vertex& vtx, float rad, int num, float w)
{
  VertexList vlist;
  vlist.resize(num);
  float ofs = num & 1 ? 0.0f : 0.5f;
  for (int i = 0; i < num; i++)
  {
    auto& v = vlist[i];
    v       = vtx;
    auto r  = (ofs + i) / (float)num;
    v.x += std::sinf(2.0f * M_PI * r) * rad;
    v.y += std::cosf(2.0f * M_PI * r) * rad;
  }
  glLineWidth(w);
  draw(vlist, GL_LINE_LOOP);
}

} // namespace Primitive2D
