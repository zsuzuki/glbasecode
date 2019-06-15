#include "font.h"
#include "codeconv.h"
#include "gl.h"
#include <ft2build.h>
#include <iostream>
#include <map>
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
GLuint     vertex_shader, fragment_shader, program;
GLint      attribute_coord, uniform_color, uniform_tex, DEPTH;
float      DrawDepth = 0.0f;

static const char* vertex_shader_text =
    "#version 120\n"
    "attribute vec4 coord;\n"
    "varying vec2 texcoord;\n"
    "uniform float Depth;\n"
    "void main(void) {\n"
    "  gl_Position = vec4(coord.xy, Depth, 1);\n"
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
using Color = Graphics::Color;

// フォント描画1つ分
struct DrawSet
{
  static constexpr float DefaultSize = 32.0f;

  FT_Face     face   = nullptr;
  float       width  = DefaultSize;
  float       height = DefaultSize;
  float       x      = 0.0f;
  float       y      = 0.0f;
  float       depth  = 0.0f;
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
  float   depth;
  float   sdepth;

public:
  WidgetImpl(const char* fontname);
  ~WidgetImpl() override = default;

  void setSize(float w, float h) override;
  void setColor(Graphics::Color c) override;
  void print(const char* msg, float x, float y) override;
  void setDepth(float d) override { depth = d; }
  void pushDepth(float d) override
  {
    sdepth = depth;
    depth  = d;
  }
  void popDepth() override { depth = sdepth; }
};

// 文字テクスチャキャッシュ
struct MyGlyph
{
  using Buffer = std::vector<uint8_t>;
  Buffer buffer;
  double width;
  double height;
  double left;
  double top;
  double ad_x;
  double ad_y;
  bool   init = false;
  GLuint tex;

  ~MyGlyph() { clear(); }

  void setup(const FT_GlyphSlot& g)
  {
    size_t sz = g->bitmap.width * g->bitmap.rows;
    buffer.resize(sz);
    memcpy(buffer.data(), g->bitmap.buffer, sz);
    width  = g->bitmap.width;
    height = g->bitmap.rows;
    left   = g->bitmap_left;
    top    = g->bitmap_top;
    ad_x   = g->advance.x;
    ad_y   = g->advance.y;
    init   = true;

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED,
                 GL_UNSIGNED_BYTE, buffer.data());
  }

  void bind() { glBindTexture(GL_TEXTURE_2D, tex); }

  void clear() { glDeleteTextures(1, &tex); }
};
std::map<int, MyGlyph> glyphs;
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
  DEPTH           = glGetUniformLocation(program, "Depth");

  glActiveTexture(GL_TEXTURE0);
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
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);
  glyphs.clear();
}

//
//
//
namespace
{
void
render(FT_Face face, const char* text, float x, float y, float sx, float sy)
{
  auto     p = text;
  char32_t ch;
  while (int r = CodeConv::U8ToU32(p, ch))
  {
    if (ch == '\0')
      break;

    p += r;
    auto& mglyph = glyphs[ch];
    if (mglyph.init == false)
    {
      if (FT_Load_Char(face, ch, FT_LOAD_RENDER))
        continue;
      mglyph.setup(face->glyph);
    }
    else
      mglyph.bind();

    float x2 = x + mglyph.left * sx;
    float y2 = -y - mglyph.top * sy;
    float w  = mglyph.width * sx;
    float h  = mglyph.height * sy;

    GLfloat box[4][4] = {
        {x2, -y2, 0, 0},
        {x2 + w, -y2, 1, 0},
        {x2, -y2 - h, 0, 1},
        {x2 + w, -y2 - h, 1, 1},
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(box), box, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    x += (mglyph.ad_x / 64) * sx;
    y += (mglyph.ad_y / 64) * sy;
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

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // render
  auto ws  = Graphics::getWindowSize();
  auto bsx = 2.0 / ws.width;
  auto bsy = 2.0 / ws.height;
  for (auto& ds : draw_set)
  {
    glUniform4fv(uniform_color, 1, (GLfloat*)&ds.color);
    glUniform1f(DEPTH, ds.depth);
    render(ds.face, ds.msg, (float)ds.x, (float)ds.y, (float)bsx, (float)bsy);
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
  depth        = DrawDepth;
  setSize(32, 32);
}

void
WidgetImpl::setSize(float w, float h)
{
  current.width  = w;
  current.height = h;
  FT_Set_Pixel_Sizes(face, 0, (FT_UInt)current.height);
}

void
WidgetImpl::setColor(const Graphics::Color c)
{
  current.color = c;
}

void
WidgetImpl::print(const char* msg, float x, float y)
{
  int  l = std::strlen(msg);
  auto p = message_buffer.size();
  message_buffer.resize(p + l + 1);
  memcpy(&message_buffer[p], msg, l + 1);

  auto nds  = current;
  nds.x     = x;
  nds.y     = y;
  nds.depth = depth;
  nds.msg   = &message_buffer[p];
  draw_set.emplace_back(nds);
}

} // namespace FontDraw
