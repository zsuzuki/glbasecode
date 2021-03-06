#pragma once

#include "checkbox.h"
#include "dialog.h"
#include "drawbox.h"
#include "font.h"
#include "gl.h"
#include "imagebutton.h"
#include "label.h"
#include "notification.h"
#include "primitive2d.h"
#include "pulldown.h"
#include "scrollbox.h"
#include "sheet.h"
#include "slidebar.h"
#include "textbox.h"
#include "textbutton.h"
#include "texture2d.h"
#include <functional>

namespace GLLib
{

// 初期化
inline FontDraw::WidgetPtr
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
  Dialog::initialize(font);
  Notification::initialize(font);
  DrawBox::initialize();
  Texture2D::initialize();
  ImageButton::initialize(font);
  Sheet::initialize();
  SlideBar::initialize();

  return font;
}

// 終了
inline void
terminate()
{
  Texture2D::terminate();
  FontDraw::terminate();
  Primitive2D::terminate();
  Graphics::terminate();
}

// レイヤー変更
inline void
bindLayer(std::string name = Parts::DefaultLayer)
{
  ScrollBox::bindLayer(name);
  TextButton::bindLayer(name);
  Label::bindLayer(name);
  CheckBox::bindLayer(name);
  TextBox::bindLayer(name);
  Pulldown::bindLayer(name);
  ImageButton::bindLayer(name);
  Sheet::bindLayer(name);
  SlideBar::bindLayer(name);
}

// フレーム内実行
template <typename Func>
inline bool
update(Func func)
{
  auto window = Graphics::setupFrame();
  if (!window)
    return false;

  Primitive2D::setup(window);
  DrawBox::setup();

  auto ret = func();

  ScrollBox::update();
  Sheet::update();
  SlideBar::update();
  TextBox::update();
  TextButton::update();
  Pulldown::update();
  Label::update();
  CheckBox::update();
  ImageButton::update();
  Dialog::update();
  Notification::update();

  Primitive2D::cleanup();
  Texture2D::update();
  FontDraw::render(window);
  Graphics::cleanupFrame();

  return ret;
}

} // namespace GLLib
