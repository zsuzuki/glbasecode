#pragma once

#include <string>
#include <vector>

namespace TextInput
{

using Buffer = std::vector<int>;

//
void start(Buffer& buff, size_t maxlength);
//
void finish();
//
bool onInput();
//
size_t getIndex();
//
std::string get();
//
void setBuffer(Buffer& buff, std::string initial);
} // namespace TextInput
