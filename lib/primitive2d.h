#pragma once

#include "gl_def.h"
#include <vector>

struct GLFWwindow;

namespace Primitive2D
{
// 頂点1つ分
struct Vertex
{
  float x = 0.0f;
  float y = 0.0f;
  float r = 1.0f;
  float g = 1.0f;
  float b = 1.0f;
  float a = 1.0f;
};
using VertexList = std::vector<Vertex>;
using Color      = Graphics::Color;

void initialize();
void setup(GLFWwindow*);
void cleanup();
void terminate();
void drawLine(const VertexList&, float w = 1.0f);
void drawCircle(const Vertex&, float rad, int num, float w = 1.0f);
void drawQuads(const VertexList&);
void drawTriangles(const VertexList&);
void drawBox(double lx, double ty, double rx, double by, const Color& col,
             bool fill);
void setDepth(float d);
void pushDepth(float d);
void popDepth();

} // namespace Primitive2D
