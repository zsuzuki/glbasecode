#pragma once

#include "parts.h"
#include <string>
#include <vector>

namespace TextInput
{
//
enum class InputStyle : int
{
  Text,
  Number,
};

using Buffer = std::vector<int>;
struct CursorOffset
{
  double left;
  double right;
};

// 入力開始
void start(Buffer& buff, size_t maxlength);
// 入力終了
void finish();
// 現在入力中か
bool onInput();
// カーソルの位置を取得
size_t getIndex();
// カーソルの位置を指定
void setIndex(size_t);
// カーソルを表示するべきX座標を取得
CursorOffset getIndexPos();
// 入力された文字列を取得(UTF-8)
std::string get();
// バッファーに指定文字列を設定する
void setBuffer(Buffer& buff, std::string initial);
// プルダウンを設定
void setPulldown(Parts::IDPtr);
// 入力スタイルの指定
void setInputStyle(InputStyle);

} // namespace TextInput
