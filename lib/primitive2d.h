#pragma once

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

void initialize();
void setup(GLFWwindow*);
void cleanup();
void terminate();
void drawLine(const VertexList&, float w = 1.0f);
void drawCircle(const Vertex&, float rad, int num, float w = 1.0f);
void drawQuads(const VertexList&);
void drawTriangles(const VertexList&);

} // namespace Primitive2D
