#pragma once

#include "parts.h"
#include <functional>
#include <memory>
#include <string>

namespace ImageButton
{
// ボタンを押したときのコールバック
using PressCallback = std::function<void()>;

//
struct Button : public Parts::ID
{
  virtual void setFocusIcon(const char*)        = 0;
  virtual void setUnFocusIcon(const char*)      = 0;
  virtual void setImageWidth(double)            = 0;
  virtual void setImageHeight(double)           = 0;
  virtual void setFocusColor(Graphics::Color)   = 0;
  virtual void setUnFocusColor(Graphics::Color) = 0;
};
using ID = std::shared_ptr<Button>;

//
void initialize();

//
void update();

//
void bindLayer(std::string = Parts::DefaultLayer);
//
void clearLayer(std::string = Parts::DefaultLayer);
//
void erase(ID);

//
ID create(const char*, double x, double y, PressCallback, bool = false);

} // namespace ImageButton
