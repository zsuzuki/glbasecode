#pragma once

#include "parts.h"
#include <memory>
#include <string>

namespace Sheet
{
static constexpr const char* DefaultLayer = "default";

//
struct Base : public Parts::ID
{
  virtual void setSize(double, double)         = 0;
  virtual void setBorderColor(Graphics::Color) = 0;
  virtual void setFillColor(Graphics::Color)   = 0;
  virtual void setDepth(float)                 = 0;
};
using ID = std::shared_ptr<Base>;

//
void initialize();

//
void update();

//
void bindLayer(std::string = DefaultLayer);
//
void clearLayer(std::string = DefaultLayer);
//
void erase(ID);

//
ID create(double x, double y, double w, double h);

} // namespace Sheet
