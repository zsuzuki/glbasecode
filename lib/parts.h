#pragma once

#include "bb.h"
#include <memory>
#include <utility>

namespace Parts
{
using UpdateResult = std::pair<bool, bool>;

//
struct ID
{
  using BBox = BoundingBox::Rect;

  BBox      bbox{};
  const ID* parent = nullptr;
  float     depth  = 0.0f;
  double    x, y;
  double    w, h;

  virtual ~ID() = default;
  virtual double getX() const { return bbox.getLeftX(); }
  virtual double getY() const { return bbox.getTopY(); }
  virtual int    getWidth() const { return bbox.getWidth(); }
  virtual int    getHeight() const { return bbox.getHeight(); }
  virtual float  getDepth() const { return depth; }
  virtual bool   getFocus() const { return false; }
  virtual void   setParent(const ID* p) { parent = p; }
  virtual double getPlacementX() const { return getX(); }
  virtual double getPlacementY() const { return getY(); }

  bool inRect(const BBox& r) const { return bbox.checkHit(r); }
  bool checkHit(double x, double y) const { return bbox.check(x, y); }

  template <typename Func>
  void update(Func func)
  {
    bool   inrect = true;
    bool   enable = true;
    double ox = 0.0, oy = 0.0;
    if (parent)
    {
      ox     = parent->getPlacementX();
      oy     = parent->getPlacementY();
      enable = parent->getFocus();
    }
    bbox = BBox{x + ox, y + oy, w, h};
    if (parent)
      inrect = parent->inRect(bbox);

    if (!inrect)
      return;

    func(enable);
  }
};

using IDPtr = std::shared_ptr<ID>;

static constexpr const char* DefaultLayer = "default";

} // namespace Parts
