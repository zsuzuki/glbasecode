#pragma once

#include "bb.h"
#include <memory>

namespace Parts
{
//
struct ID
{
  virtual ~ID()                    = default;
  virtual double getX() const      = 0;
  virtual double getY() const      = 0;
  virtual int    getWidth() const  = 0;
  virtual int    getHeight() const = 0;
  virtual float  getDepth() const { return 0.0f; }
  virtual bool   getFocus() const { return false; }

  virtual void   setParent(const ID*) {}
  virtual double getPlacementX() const { return getX(); }
  virtual double getPlacementY() const { return getY(); }
  virtual bool   inRect(const BoundingBox::Rect&) const { return true; }
};

using IDPtr = std::shared_ptr<ID>;

} // namespace Parts
