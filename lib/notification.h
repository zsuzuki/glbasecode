#pragma once

#include "font.h"
#include "gl_def.h"
#include <memory>
#include <string>

namespace Notification
{
struct Msg
{
  virtual ~Msg()                               = default;
  virtual void setBorderColor(Graphics::Color) = 0;
  virtual void setFillColor(Graphics::Color)   = 0;
  virtual void setFontColor(Graphics::Color)   = 0;
};
using ID = std::shared_ptr<Msg>;

// 通知の表示
// msg: 表示するメッセージ
// dispms: 表示する時間(msec)
ID notify(std::string msg, int dispms = 3000);

//
void initialize(FontDraw::WidgetPtr);

//
void update();

} // namespace Notification
