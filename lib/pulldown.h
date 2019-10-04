#pragma once

#include "font.h"
#include "gl_def.h"
#include "parts.h"
#include <functional>
#include <string>
#include <vector>

namespace Pulldown
{
using Selected = std::function<void(int, const std::string&)>;
//
struct Base : public Parts::ID
{
  virtual bool   setFilter(std::string) = 0;
  virtual size_t getIndex() const       = 0;
  virtual void   open()                 = 0;
  virtual void   close()                = 0;
  virtual bool   isOpened() const       = 0;
  virtual void   setSelected(Selected)  = 0;
  virtual void   setChanged(Selected)   = 0;
};
using ID = std::shared_ptr<Base>;

using List = std::vector<std::string>;

//
void initialize(FontDraw::WidgetPtr font);
//
void update();
// 要素作成
// l リスト
// nb_disp 画面に表示する個数
ID create(List&& l, size_t nb_disp);
//
void bindLayer(std::string layer = Parts::DefaultLayer);
//
void clearLayer(std::string layer = Parts::DefaultLayer);
//
void erase(ID);

} // namespace Pulldown
