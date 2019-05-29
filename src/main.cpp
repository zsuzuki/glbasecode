#include "gl.h"
#include <iostream>

#include "font.h"
#include "primitive2d.h"

namespace
{
// key check
void
key_callback(int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    Graphics::finish();
}
// file drag&drop
void
drop_file(int num, const char** paths)
{
  for (int i = 0; i < num; i++)
  {
    std::cout << paths[i] << std::endl;
  }
}
// mouse push
void
mouse_button(int button, int action, int mods)
{
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    std::cout << "Left Button" << std::endl;
}
} // namespace

//
//
//
int
main(int argc, char** argv)
{
  int w = 1024;
  int h = 1024;
  if (!Graphics::initialize("Sample", w, h))
    return 1;

  Graphics::setKeyCallback(key_callback);
  Graphics::setDropCallback(drop_file);
  Graphics::setMouseButtonCallback(mouse_button);

  {
    extern void testInit();
    testInit();
  }

  Primitive2D::initialize();
  FontDraw::initialize();

  auto font = FontDraw::create("font/SourceHanCodeJP-Normal.otf");

  // フレームループ
  while (auto window = Graphics::setupFrame())
  {
    {
      extern void testSetup();
      extern void testRender(GLFWwindow*);
      testSetup();
      testRender(window);
    }

    Primitive2D::setup(window);
    {
      static const Primitive2D::VertexList vl = {
          {-0.4f, -0.4f, 1.0f, 0.0f, 0.0f},
          {0.4f, -0.4f, 0.0f, 1.0f, 0.0f},
          {0.4f, 0.4f, 0.0f, 0.0f, 1.0f},
          {-0.4f, 0.4f, 1.0f, 1.0f, 1.0f},
      };
      Primitive2D::drawLine(vl, 4.0f);
      static const Primitive2D::Vertex v = {0.0f, 0.0f, 0.5f, 1.0f, 1.0f};
      Primitive2D::drawCircle(v, 0.5f, 5, 8.0f);
      Primitive2D::drawCircle(v, 0.52f, 6, 4.0f);
      Primitive2D::drawCircle(v, 0.54f, 8, 2.0f);
      Primitive2D::drawCircle(v, 0.56f, 16);
    }
    Primitive2D::cleanup();

    font->setColor(0.0f, 1.0f, 0.0f);
    font->print("こんにちは、世界", -0.3f, -0.3f);
    font->setColor(0.8f, 0.8f, 1.0f);
    font->print("Status: Echo", -0.98f, -1.0f);
    font->print("Title: Top", -0.98f, 1.0f - (32.0f / 480.0f));
    FontDraw::render(window);

    Graphics::cleanupFrame();
  }

  FontDraw::terminate();
  Primitive2D::terminate();

  Graphics::terminate();

  return 0;
}
