#pragma once

#include "checkbox.h"
#include "font.h"
#include "gl.h"
#include "label.h"
#include "primitive2d.h"
#include "pulldown.h"
#include "scrollbox.h"
#include "textbox.h"
#include "textbutton.h"
#include <functional>

namespace GLLib
{

// 初期化
FontDraw::WidgetPtr
initialize(const char* appname, const char* fontname, int w, int h)
{
  if (!Graphics::initialize(appname, w, h))
    return FontDraw::WidgetPtr();

  FontDraw::initialize();
  auto font = FontDraw::create(fontname);

  Primitive2D::initialize();
  TextButton::initialize(font);
  TextBox::initialize(font);
  Label::initialize(font);
  CheckBox::initialize(font);
  Pulldown::initialize(font);

  return font;
}

// 終了
void
terminate()
{
  FontDraw::terminate();
  Primitive2D::terminate();
  Graphics::terminate();
}

// レイヤー変更
void
bindLayer(std::string name = "default")
{
  ScrollBox::bindLayer(name);
  TextButton::bindLayer(name);
  Label::bindLayer(name);
  CheckBox::bindLayer(name);
  TextBox::bindLayer(name);
}

// フレーム内実行
bool
update(std::function<bool()> func)
{
  auto window = Graphics::setupFrame();
  if (!window)
    return false;

  Primitive2D::setup(window);

  auto ret = func();

  ScrollBox::update();
  TextBox::update();
  TextButton::update();
  Pulldown::update();
  Label::update();
  CheckBox::update();

  Primitive2D::cleanup();
  FontDraw::render(window);
  Graphics::cleanupFrame();

  return ret;
}

} // namespace GLLib
