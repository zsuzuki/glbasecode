#include "font.h"
#include <GLFW/glfw3.h>
#include <OpenGL/gl.h>
#include <ft2build.h>
#include <iostream>
#include <vector>
#include FT_FREETYPE_H

//
// 以下のサイトを参考にした
// 描画部分はほぼそのまま使っている
// https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_01
//
namespace FontDraw
{

namespace
{
FT_Library ft;
GLuint     vbo;
GLuint     vertex_shader, fragment_shader, program, tex;
GLint      attribute_coord, uniform_color, uniform_tex;

static const char* vertex_shader_text =
    "#version 120\n"
    "attribute vec4 coord;\n"
    "varying vec2 texcoord;\n"
    "void main(void) {\n"
    "  gl_Position = vec4(coord.xy, 0, 1);\n"
    "  texcoord    = coord.zw;\n"
    "}";
static const char* fragment_shader_text =
    "#version 120\n"
    "varying vec2 texcoord;\n"
    "uniform sampler2D tex;\n"
    "uniform vec4 color;\n"
    "void main(void) {\n"
    "  gl_FragColor = vec4(1, 1, 1, texture2D(tex, texcoord).r) * color;\n"
    "}";

// 色
struct Color
{
  float r = 1.0f;
  float g = 1.0f;
  float b = 1.0f;
  float a = 1.0f;

  void set(float rr, float gg, float bb, float aa)
  {
    r = rr;
    g = gg;
    b = bb;
    a = aa;
  }
};

// フォント描画1つ分
struct DrawSet
{
  static constexpr float DefaultSize = 32.0f;

  FT_Face     face   = nullptr;
  float       width  = DefaultSize;
  float       height = DefaultSize;
  float       x      = 0.0f;
  float       y      = 0.0f;
  Color       color{};
  const char* msg = nullptr;
};
std::vector<char>    message_buffer;
std::vector<DrawSet> draw_set;

//
// ウィジェット実装
//
class WidgetImpl : public Widget
{
  FT_Face face;
  DrawSet current;
  bool    valid;

public:
  WidgetImpl(const char* fontname);
  ~WidgetImpl() override = default;

  void setSize(float w, float h) override;
  void setColor(float r, float g, float b, float a) override;
  void print(const char* msg, float x, float y) override;
};

} // namespace

//
//
//
std::shared_ptr<Widget>
create(const char* fontname)
{
  return std::make_shared<WidgetImpl>(fontname);
}

//
//
//
bool
initialize()
{
  if (FT_Init_FreeType(&ft))
  {
    std::cerr << "Could not init freetype library" << std::endl;
    return false;
  }

  glGenBuffers(1, &vbo);
  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_text, nullptr);
  glCompileShader(vertex_shader);
  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_text, nullptr);
  glCompileShader(fragment_shader);
  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);
  uniform_tex     = glGetUniformLocation(program, "tex");
  uniform_color   = glGetUniformLocation(program, "color");
  attribute_coord = glGetAttribLocation(program, "coord");

  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &tex);
  glUniform1i(uniform_tex, 0);

  message_buffer.reserve(10 * 1024);
  message_buffer.resize(0);
  draw_set.reserve(1024);
  draw_set.resize(0);

  return true;
}

//
//
//
void
terminate()
{
  glDeleteBuffers(1, &vbo);
  glDeleteProgram(program);
  glDeleteTextures(1, &tex);
}

//
//
//
namespace
{
void
render(FT_Face face, const char* text, float x, float y, float sx, float sy)
{
  const char* p;

  for (p = text; *p; p++)
  {
    if (FT_Load_Char(face, *p, FT_LOAD_RENDER))
      continue;

    FT_GlyphSlot g = face->glyph;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, g->bitmap.width, g->bitmap.rows, 0,
                 GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);

    float x2 = x + g->bitmap_left * sx;
    float y2 = -y - g->bitmap_top * sy;
    float w  = g->bitmap.width * sx;
    float h  = g->bitmap.rows * sy;

    GLfloat box[4][4] = {
        {x2, -y2, 0, 0},
        {x2 + w, -y2, 1, 0},
        {x2, -y2 - h, 0, 1},
        {x2 + w, -y2 - h, 1, 1},
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(box), box, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    x += (g->advance.x / 64) * sx;
    y += (g->advance.y / 64) * sy;
  }
}
} // namespace

void
render(GLFWwindow* window)
{
  // setup
  glUseProgram(program);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glEnableVertexAttribArray(attribute_coord);
  glVertexAttribPointer(attribute_coord, 4, GL_FLOAT, GL_FALSE, 0, 0);

  glEnable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // render
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  float bsx = 2.0f / (float)width;
  float bsy = 2.0f / (float)height;
  for (auto& ds : draw_set)
  {
    glUniform4fv(uniform_color, 1, (GLfloat*)&ds.color);
    render(ds.face, ds.msg, ds.x, ds.y, bsx, bsy);
  }

  // cleanup
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDisableVertexAttribArray(attribute_coord);
  glDisable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);

  message_buffer.resize(0);
  draw_set.resize(0);
}

//
// Widget
//

//
//
//
WidgetImpl::WidgetImpl(const char* fontname)
{
  if (FT_New_Face(ft, fontname, 0, &face))
  {
    std::cerr << "Could not open font: " << fontname << std::endl;
    valid = false;
    return;
  }

  valid        = true;
  current.face = face;
  setSize(32, 32);
}

void
WidgetImpl::setSize(float w, float h)
{
  current.width  = w;
  current.height = h;
  FT_Set_Pixel_Sizes(face, 0, current.height);
}

void
WidgetImpl::setColor(float r, float g, float b, float a)
{
  current.color.set(r, g, b, a);
}

void
WidgetImpl::print(const char* msg, float x, float y)
{
  int  l = std::strlen(msg);
  auto p = message_buffer.size();
  message_buffer.resize(p + l + 1);
  memcpy(&message_buffer[p], msg, l + 1);

  auto nds = current;
  nds.x    = x;
  nds.y    = y;
  nds.msg  = &message_buffer[p];
  draw_set.emplace_back(nds);
}

} // namespace FontDraw
