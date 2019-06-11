#include <exec.h>
#include <font.h>
#include <gl.h>
#include <iostream>
#include <primitive2d.h>
#include <textbox.h>
#include <textbutton.h>

namespace
{

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
// scroll
double scr_x, scr_y;
void
scroll(double xofs, double yofs)
{
  scr_x = xofs;
  scr_y = yofs;
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
  Graphics::setScrollCallback(scroll);

  Primitive2D::initialize();
  FontDraw::initialize();

  auto font = FontDraw::create("font/SourceHanCodeJP-Normal.otf");
  TextButton::initialize(font);
  TextBox::initialize(font);

  // put text-boxs
  TextBox::create("Text", 50, 500, 300, 50);
  TextBox::create("Sample", 50, 560, 300, 50);
  auto tb = TextBox::create("", 50, 620, 300, 80);
  tb->setPlaceHolder("example");

  Exec::IStreamPtr inf;
  Exec::HandlePtr  handle;
  Exec::setup();
  Exec::setCloseFunc([]() { std::cout << "execute done." << std::endl; });

  // put text buttons
  TextButton::setButton("Input", 150, 260,
                        []() { TextButton::bindLayer("Submit"); });
  bool           newbtn = true;
  TextButton::ID tbid;
  tbid = TextButton::setButton("Test1", 150, 330,
                               [&]() {
                                 std::cout << "Test1" << std::endl;
                                 if (newbtn)
                                 {
                                   int x = tbid->getWidth() + 150;
                                   int y = tbid->getHeight() + 330;
                                   TextButton::setButton("Test2", x, y, []() {
                                     std::cout << "Test2" << std::endl;
                                   });
                                   newbtn = false;
                                 }
                               },
                               true);
  TextButton::bindLayer("Submit");
  TextButton::setButton("Submit", 150, 260,
                        [&]() {
                          if (Exec::now_exec == false)
                          {
                            auto cmd = tb->getText();
                            handle   = Exec::run(cmd.c_str(), ".", nullptr);
                            if (!handle)
                            {
                              std::cout << "execute error" << std::endl;
                              return;
                            }
                            inf = Exec::buildInStream(handle);
                          }
                          TextButton::bindLayer();
                        },
                        true);
  TextButton::bindLayer();

  // フレームループ
  while (auto window = Graphics::setupFrame())
  {
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

    font->setColor({0.0f, 1.0f, 0.0f});
    font->print("こんにちは、世界", -0.3f + scr_x * 0.01f,
                -0.3f + scr_y * 0.01f);
    font->setColor({0.8f, 0.8f, 1.0f});
    font->print("Status: Echo", -0.98f, -1.0f);
    font->print("Title: Top", -0.98f, 1.0f - (32.0f / 480.0f));
    TextBox::update();
    TextButton::update();
    Primitive2D::cleanup();
    FontDraw::render(window);

    if (inf)
    {
      if (inf->eof())
      {
        inf.reset();
      }
      else
      {
        std::string b;
        (*inf) >> b;
        std::cout << "in: " << b << std::endl;
      }
    }

    Graphics::cleanupFrame();
  }

  FontDraw::terminate();
  Primitive2D::terminate();

  Graphics::terminate();

  return 0;
}
