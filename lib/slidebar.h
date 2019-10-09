#pragma once

#include "parts.h"
#include <functional>
#include <memory>
#include <string>

namespace SlideBar
{
//
using Changed = std::function<void(double)>;

//
enum class Type : int
{
  Real,
  Integer,
};

//
struct Bar : public Parts::ID
{
  virtual double getNumber() const                 = 0;
  virtual void   setNumber(double n)               = 0;
  virtual void   setMinMax(double min, double max) = 0;
  virtual void   setStep(double s)                 = 0;
  virtual void   setChanged(Changed)               = 0;
  virtual void   setNumberType(Type)               = 0;
};

using ID = std::shared_ptr<Bar>;

//
void initialize();

//
ID create(double x, double y, double w, double h);

//
void bindLayer(std::string layer = Parts::DefaultLayer);
//
void clearLayer(std::string layer = Parts::DefaultLayer);
//
void erase(ID);
//
void update();

} // namespace SlideBar
