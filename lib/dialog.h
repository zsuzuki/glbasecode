#pragma once

#include "font.h"
#include "parts.h"
#include <functional>

namespace Dialog
{
//
using DecideFunc = std::function<void(bool)>;

//
struct Base : public Parts::ID
{
  virtual void setOK(DecideFunc)     = 0;
  virtual void setCancel(DecideFunc) = 0;
};

using ID = std::shared_ptr<Base>;

//
void initialize(FontDraw::WidgetPtr font);

//
ID create(std::string msg, bool need_cancel = false);

// 指定したダイアログをオープンする
// 他のパーツと違い、明示的にオープンを指定しないといけない
// そして、同時に一つしか実行できない
// @param ID オープンするダイアログ
// @return trueならオープンできた
bool open(ID);

// ボタンでは無く強制的にダイアログを閉じる(キャンセルが呼ばれる)
void close();

//
void update();

} // namespace Dialog