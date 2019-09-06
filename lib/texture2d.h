#pragma once

#include "gl_def.h"
#include <memory>

namespace Texture2D
{
//
enum class Align : int
{
  LeftTop,
  Left,
  LeftBottom,
  CenterTop,
  Center,
  CenterBottom,
  RightTop,
  Right,
  RightBottom,
};

//
struct Image
{
  virtual ~Image() = default;
  //
  virtual int getWidth() const  = 0;
  virtual int getHeight() const = 0;
};

using ImagePtr = std::shared_ptr<Image>;

//
struct DrawSet
{
  using Color = Graphics::Color;

  ImagePtr image;
  double   x, y;
  double   width, height;
  float    depth;
  double   rotate;
  Align    align;
  Color    color;
};

//
void initialize();

//
void terminate();

//
void update();

// イメージオブジェクトの作成
// const char*: ファイル名(png)
ImagePtr create(const char*);

//
void draw(const DrawSet& di);

//
void setDrawArea(double x, double y, double w, double h);

//
void clearDrawArea();

} // namespace Texture2D
