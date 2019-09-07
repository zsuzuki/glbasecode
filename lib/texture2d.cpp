#include "texture2d.h"
#include "gl.h"
#include <cmath>
#include <exception>
#include <iostream>
#include <png.h>
#include <vector>

namespace Texture2D
{

namespace
{
using Color    = Graphics::Color;
using DrawArea = Graphics::DrawArea;

// shader
const char* vtx_sh_s = "#version 120\n"
                       "attribute vec4 coord;\n"
                       "varying vec2 texcoord;\n"
                       "uniform float Depth;\n"
                       "void main(void) {\n"
                       "  gl_Position = vec4(coord.xy, Depth, 1);\n"
                       "  texcoord    = coord.zw;\n"
                       "}";
const char* frag_sh_s = "#version 120\n"
                        "varying vec2 texcoord;\n"
                        "uniform sampler2D tex;\n"
                        "uniform vec4 color;\n"
                        "void main(void) {\n"
                        "  gl_FragColor = texture2D(tex, texcoord) * color;\n"
                        "}";

GLuint   vb_obj;
GLuint   vtx_sh, frg_sh, sh_prog;
GLint    attr_coord, uni_col, uni_tex, uni_depth;
DrawArea draw_area{};

//
struct ImageImpl : public Image
{
  int    width  = 0;
  int    height = 0;
  GLuint tex_id = 0;

  ~ImageImpl() { clear(); };
  //
  int getWidth() const override { return width; }
  int getHeight() const override { return height; }

  void createRGB(void* buffer, int ch)
  {
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    auto t = (ch == 4) ? GL_RGBA : (ch == 3) ? GL_RGB : GL_LUMINANCE_ALPHA;
    glTexImage2D(GL_TEXTURE_2D, 0, t, width, height, 0, t, GL_UNSIGNED_BYTE,
                 buffer);
  }
  void bind() { glBindTexture(GL_TEXTURE_2D, tex_id); }
  void clear() { glDeleteTextures(1, &tex_id); }
};

struct DrawSetIntr : public DrawSet
{
  DrawArea da;
};
std::vector<DrawSetIntr> draw_list;

} // namespace

//
void
initialize()
{
  glGenBuffers(1, &vb_obj);
  vtx_sh = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vtx_sh, 1, &vtx_sh_s, nullptr);
  glCompileShader(vtx_sh);
  frg_sh = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frg_sh, 1, &frag_sh_s, nullptr);
  glCompileShader(frg_sh);
  sh_prog = glCreateProgram();
  glAttachShader(sh_prog, vtx_sh);
  glAttachShader(sh_prog, frg_sh);
  glLinkProgram(sh_prog);
  attr_coord = glGetAttribLocation(sh_prog, "coord");
  uni_tex    = glGetUniformLocation(sh_prog, "tex");
  uni_col    = glGetUniformLocation(sh_prog, "color");
  uni_depth  = glGetUniformLocation(sh_prog, "Depth");

  draw_list.reserve(1000);
  draw_list.resize(0);
}

//
void
terminate()
{
  glDeleteBuffers(1, &vb_obj);
  glDeleteProgram(sh_prog);
  glDeleteShader(vtx_sh);
  glDeleteShader(frg_sh);
}

//
void
setDrawArea(double x, double y, double w, double h)
{
  draw_area.x = x;
  draw_area.y = y;
  draw_area.w = w;
  draw_area.h = h;
  draw_area.e = true;
}

//
void
clearDrawArea()
{
  draw_area.e = false;
}

//
void
update()
{
  glUseProgram(sh_prog);
  glBindBuffer(GL_ARRAY_BUFFER, vb_obj);
  glEnableVertexAttribArray(attr_coord);
  glVertexAttribPointer(attr_coord, 4, GL_FLOAT, GL_FALSE, 0, 0);

  glEnable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE0);
  glUniform1i(uni_tex, 0);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  auto da  = DrawArea{};
  auto ws  = Graphics::getWindowSize();
  auto asp = ws.width / ws.height;
  for (const auto& dset : draw_list)
  {
    auto image = dynamic_cast<ImageImpl*>(dset.image.get());
    if (!image)
      continue;
    glUniform4fv(uni_col, 1, (GLfloat*)&dset.color);
    glUniform1f(uni_depth, dset.depth);

    static const GLfloat align_scale[][4] = {
        {0.0f, 1.0f, 0.0f, -1.0f},  // Left Top
        {0.0f, 1.0f, 0.5f, -0.5f},  // Left
        {0.0f, 1.0f, 1.0f, 0.0f},   // Left Bottom
        {-0.5f, 0.5f, 0.0f, -1.0f}, // Center Top
        {-0.5f, 0.5f, 0.5f, -0.5f}, // Center
        {-0.5f, 0.5f, 1.0f, 0.0f},  // Center Bottom
        {-1.0f, 0.0f, 0.0f, -1.0f}, // Right Top
        {-1.0f, 0.0f, 0.5f, -0.5f}, // Right
        {-1.0f, 0.0f, 1.0f, 0.0f},  // Right Bottom
    };
    int     al_ofs     = static_cast<int>(dset.align);
    auto    al_list    = align_scale[al_ofs];
    GLfloat left       = dset.width * al_list[0];
    GLfloat right      = dset.width * al_list[1];
    GLfloat top        = dset.height * al_list[2];
    GLfloat bottom     = dset.height * al_list[3];
    GLfloat dbox[4][4] = {
        {left, top, 0, 0},
        {right, top, 1, 0},
        {left, bottom, 0, 1},
        {right, bottom, 1, 1},
    };
    if (dset.rotate != 0.0)
    {
      auto c = std::cos(dset.rotate);
      auto s = std::sin(dset.rotate);
      for (auto& p : dbox)
      {
        auto x = p[0];
        auto y = p[1];
        p[0]   = x * c + y * s + dset.x;
        p[1]   = (-x * s + y * c) * asp + dset.y;
      }
    }
    else
    {
      for (auto& p : dbox)
      {
        p[0] += dset.x;
        p[1] = p[1] * asp + dset.y;
      }
    }
    image->bind();
    dset.da.set(da);
    da = dset.da;
    glBufferData(GL_ARRAY_BUFFER, sizeof(dbox), dbox, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }
  Graphics::disableScissor();

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDisableVertexAttribArray(attr_coord);
  glDisable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);

  draw_list.resize(0);
}

//
ImagePtr
create(const char* fname)
{
  // ファイル読み込み失敗例外
  class ex : public std::exception
  {
    const char* msg = "error";

  public:
    png_structp png  = nullptr;
    png_infop   info = nullptr;

    ex(const char* m, png_structp p = nullptr, png_infop i = nullptr)
    {
      msg  = m;
      png  = p;
      info = i;
    }
    ~ex() = default;
    const char* what() const noexcept override { return msg; }
  };

  // ファイルオープン
  FILE* fp = fopen(fname, "rb");
  if (!fp)
    return ImagePtr{};

  auto res = ImagePtr{};
  try
  {
    // 読み込み(これ以降はエラーは例外処理)
    png_byte header[8];
    auto     hsize = sizeof(header);
    if (fread(header, 1, hsize, fp) != hsize)
      throw(ex{"header read failed"});

    auto is_png = !png_sig_cmp(header, 0, hsize);
    if (!is_png)
      throw(ex{"not png file"});

    auto png_ptr = png_create_read_struct(
        PNG_LIBPNG_VER_STRING, (png_voidp) nullptr, nullptr, nullptr);
    if (!png_ptr)
      throw(ex{"read struct create failed"});

    auto info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
      throw(ex{"info struct create failed"});

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, hsize);
    png_read_info(png_ptr, info_ptr);

    auto w = png_get_image_width(png_ptr, info_ptr);
    auto h = png_get_image_height(png_ptr, info_ptr);

    auto type = png_get_color_type(png_ptr, info_ptr);
    if (type != PNG_COLOR_TYPE_RGB && type != PNG_COLOR_TYPE_RGB_ALPHA &&
        type != PNG_COLOR_TYPE_GRAY_ALPHA)
      throw(ex{"not support format"});

    auto rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    auto channels = (int)png_get_channels(png_ptr, info_ptr);

    std::vector<png_byte>  img(rowbytes * h);
    std::vector<png_bytep> row_p(h);

    for (png_uint_32 i = 0; i < h; i++)
      row_p[i] = &img[i * rowbytes];
    png_read_image(png_ptr, row_p.data());

    auto image    = std::make_shared<ImageImpl>();
    image->width  = w;
    image->height = h;
    image->createRGB(img.data(), channels);
    res = image;

    png_read_end(png_ptr, nullptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
  }
  catch (const ex& e)
  {
    // エラー
    std::cerr << "png read error: " << e.what() << std::endl;
    auto p = e.png;
    auto i = e.info;
    png_destroy_read_struct(&p, &i, nullptr);
  }

  fclose(fp);
  return res;
}

//
void
draw(const DrawSet& di)
{
  DrawSetIntr dsi;
  DrawSet&    dst = dsi;
  dst             = di;
  dsi.da          = draw_area;
  draw_list.emplace_back(dsi);
}

} // namespace Texture2D
