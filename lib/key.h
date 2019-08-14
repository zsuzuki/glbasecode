#pragma once

#include <bitset>

namespace Key
{
// サポートされるキー
enum class Code : int
{
  None,
  Up,
  Down,
  Right,
  Left,
  Enter,
  Tab,
  BackSpace,
  Delete,
  PageUp,
  PageDown,
  Home,
  End,
  Esc,
  Insert,
  Space,
};

// 入力情報クラス
class Input
{
public:
  virtual ~Input() = default;
  //
  virtual bool onKey(Code) const = 0;
  virtual Code getRepeat() const = 0;
  virtual bool onShift() const   = 0;
  virtual bool onCtrl() const    = 0;
  virtual bool onAlt() const     = 0;
};

} // namespace Key
