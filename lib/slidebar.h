#pragma once

#include "parts.h"
#include <memory>

namespace SlideBar
{

struct Base : public Parts::ID
{
};

using ID = std::shared_ptr<Base>;

//
void initialize();

//
ID create();

//
void bindLayer(std::string layer = Parts::DefaultLayer);
//
void clearLayer(std::string layer = Parts::DefaultLayer);
//
void erase(ID);
//
void update();

} // namespace SlideBar