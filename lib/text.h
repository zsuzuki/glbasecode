#pragma once

#include <string>
#include <vector>

namespace TextInput
{

using Buffer = std::vector<int>;
struct CursorOffset
{
  double left;
  double right;
};

//
void start(Buffer& buff, size_t maxlength);
//
void finish();
//
bool onInput();
//
size_t getIndex();
//
void setIndex(size_t);
//
CursorOffset getIndexPos();
//
std::string get();
//
void setBuffer(Buffer& buff, std::string initial);
} // namespace TextInput
