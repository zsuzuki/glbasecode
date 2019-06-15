#pragma once

#include "font.h"
#include "gl_def.h"
#include "parts.h"
#include <string>

namespace Label
{
static constexpr const char* DefaultLayer = "default";

using ID    = Parts::IDPtr;
using Color = Graphics::Color;

//
void initialize(FontDraw::WidgetPtr font);
//
ID create(std::string str, double x, double y, Color fg, Color bg);
//
void bindLayer(std::string layer = DefaultLayer);
//
void clearLayer(std::string layer = DefaultLayer);
//
void erase(ID);
//
void update();

} // namespace Label
