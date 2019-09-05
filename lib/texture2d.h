#pragma once

#include "gl_def.h"
#include <memory>

namespace Texture2D
{
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
void initialize();

//
void terminate();

//
void update();

// イメージオブジェクトの作成
// const char*: ファイル名(png)
ImagePtr create(const char*);

//
void draw(ImagePtr, double x, double y, double w, double h, float d = 0.0f);

//
void setFadeColor(Graphics::Color);

//
void setDrawArea(double x, double y, double w, double h);

//
void clearDrawArea();

} // namespace Texture2D
