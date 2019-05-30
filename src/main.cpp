#include "gl.h"
#include <iostream>

#include "font.h"
#include "primitive2d.h"
#include "text.h"
#include "textbutton.h"

namespace
{
TextInput::Buffer text_buffer;

// key check
void
key_callback(int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    Graphics::finish();
  // else if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
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

  TextInput::setBuffer(text_buffer, "Hello");
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
  TextButton::initialize(font);

  TextButton::setButton("Input", 150, 260, []() {
    TextInput::start(text_buffer, 20);
    TextButton::bindLayer("Submit");
  });
  bool newbtn = true;
  TextButton::setButton("Test1", 150, 330, [&]() {
    std::cout << "Test1" << std::endl;
    if (newbtn)
    {
      TextButton::setButton("Test2", 400, 330,
                            []() { std::cout << "Test2" << std::endl; });
      newbtn = false;
    }
  });
  TextButton::bindLayer("Submit");
  TextButton::setButton("Submit", 150, 260, []() {
    TextInput::finish();
    TextButton::bindLayer();
  });
  TextButton::bindLayer();

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

    font->setColor(0.0f, 1.0f, 0.0f);
    font->print("こんにちは、世界", -0.3f, -0.3f);
    font->setColor(0.8f, 0.8f, 1.0f);
    font->print("Status: Echo", -0.98f, -1.0f);
    font->print("Title: Top", -0.98f, 1.0f - (32.0f / 480.0f));
    if (TextInput::onInput())
    {
      auto p   = Graphics::calcLocate(50.0, 150.0);
      auto tp  = Graphics::calcLocate(60.0, 190.0);
      auto ofs = TextInput::getIndexPos();
      auto l1  = Graphics::calcLocate(60.0 + ofs.left, 190.0, true);
      auto l2  = Graphics::calcLocate(60.0 + ofs.right, 190.0, true);
      font->setColor(1.0f, 1.0f, 1.0f);
      font->print("Input:", p.x, p.y);
      font->print(TextInput::get().c_str(), tp.x, tp.y);
      static Primitive2D::VertexList ul = {
          {0.0f, 0.0f, 0.0f, 1.0f, 0.0f},
          {0.0f, 0.0f, 0.0f, 1.0f, 0.0f},
      };
      ul[0].x = l1.x;
      ul[0].y = l1.y;
      ul[1].x = l2.x;
      ul[1].y = l2.y;
      Primitive2D::drawLine(ul, 4.0f);
    }
    TextButton::update();
    Primitive2D::cleanup();
    FontDraw::render(window);

    Graphics::cleanupFrame();
  }

  FontDraw::terminate();
  Primitive2D::terminate();

  Graphics::terminate();

  return 0;
}
