#include <exec.h>
#include <gllib.h>
#include <iostream>
#include <list>

namespace
{
int  Width    = 1024;
int  Height   = 1024;
bool DispPrim = true;

// file drag&drop
void
drop_file(int num, const char** paths)
{
  for (int i = 0; i < num; i++)
  {
    std::cout << paths[i] << std::endl;
  }
}

//
void
setupMenu1()
{
  Label::create("SETTINGS", 100, 80, Graphics::Green, Graphics::Gray);
  TextButton::setButton("Return", 100, 150, []() { GLLib::bindLayer(); });

  TextButton::setButton("Disp Primitive", 200, 300,
                        []() { DispPrim = !DispPrim; });
  auto tb = TextBox::create("", 200, 450, 400);
  tb->setMaxLength(16);
  tb->setPlaceHolder("filename");
}

//
void
setupMenu2()
{
  Label::create("EDITOR SCREEN", 100, 80, Graphics::Red, Graphics::Gray);
  TextButton::setButton("Return", 100, 150, []() { GLLib::bindLayer(); });

  auto SBox = ScrollBox::create();
  SBox->set(300, 300, 700, 700);
  SBox->drawSheet(true, {0.1f, 0.4f, 0.5f, 0.4f});
  SBox->setDepth(-0.1f);
  SBox->setScrollConstraint(true, false);

  double by = 100.0;
  for (int i = 0; i < 20; i++)
  {
    double x = 60.0;

    auto cb = CheckBox::create("✓", x, by, false);
    cb->setOnColor(Graphics::Green);
    cb->setOffText("Off");
    x += cb->getWidth() + 50.0;
    SBox->append(cb);

    std::string l = "Item";
    l += std::to_string(i + 1);
    auto p = TextButton::setButton(l, x, by, []() {});
    by += p->getHeight() + 10;
    SBox->append(p);
  }
  auto tfg = Graphics::Yellow;
  auto tbg = Graphics::Blue;
  SBox->append(Label::create("Scroll Menu", 150, 20, tfg, tbg));
}

//
void
setupMenu3()
{
  Label::create("INFORMATION", 100, 80, Graphics::Cyan, Graphics::Gray);
  TextButton::setButton("Return", 100, 150, []() { GLLib::bindLayer(); });

  Pulldown::List pl{"PULLDOWN 1", "PULLDOWN 2", "PULLDOWN 3", "PULLDOWN 4",
                    "TEST 5",     "TEST 6",     "SAMPLE 7",   "SAMPLE 8"};
  auto           pd = Pulldown::create(std::move(pl), 5);
  auto           tb = TextButton::setButton("Pulldown", 200, 300, []() {});
  tb->setPulldown(pd);
  pd->setSelected(
      [tb](int idx) { tb->setCaption("Select " + std::to_string(idx)); });
  pd->setChanged(
      [tb](int idx) { tb->setCaption("Change " + std::to_string(idx)); });
}

//
void
setup()
{
  Graphics::setDropCallback(drop_file);

  // top menu
  {
    using namespace TextButton;
    setDefaultColor(ColorType::UnFocusFont, Graphics::Orange);
    setDefaultColor(ColorType::FocusFont, Graphics::Orange);
    setDefaultColor(ColorType::FocusBG, Graphics::Gray);
    setDefaultColor(ColorType::PressFont, Graphics::Green);
    auto eb = setButton("Exit", Width - 150, 50, []() { Graphics::finish(); });
    eb->setColor(ColorType::UnFocusFont, Graphics::Red);
    eb->setColor(ColorType::FocusFont, Graphics::Red);
    eb->setColor(ColorType::FocusBG, Graphics::Sepia);
    eb->setColor(ColorType::PressBG, Graphics::Red);
    eb->setColor(ColorType::PressFont, Graphics::Black);
  }
  for (int i = 0; i < 3; i++)
  {
    int  idx = i + 1;
    auto t   = std::string("MENU") + std::to_string(idx);
    GLLib::bindLayer();
    TextButton::setButton(t, 150, 150 + i * 80, [t]() { GLLib::bindLayer(t); });
    GLLib::bindLayer(t);
    switch (idx)
    {
    case 1:
      setupMenu1();
      break;
    case 2:
      setupMenu2();
      break;
    case 3:
      setupMenu3();
      break;
    default:
      break;
    }
  }
}

//
bool
onUpdate(FontDraw::WidgetPtr font)
{
  // プリミティブを描画
  if (DispPrim)
  {
    static const Primitive2D::VertexList vl = {
        {-0.4f, -0.4f, 1.0f, 0.0f, 0.0f},
        {0.4f, -0.4f, 0.0f, 1.0f, 0.0f},
        {0.4f, 0.4f, 0.0f, 0.0f, 1.0f},
        {-0.4f, 0.4f, 1.0f, 1.0f, 1.0f},
    };
    Primitive2D::drawQuads(vl);
    static const Primitive2D::Vertex v = {0.0f, 0.0f, 0.5f, 1.0f, 1.0f};
    Primitive2D::drawCircle(v, 0.5f, 32, 8.0f);
  }

  // グラフィック座標系で毎フレーム描画
  font->setColor(Graphics::Green);
  font->print("こんにちは、世界", -0.3f, -0.3f);
  font->setColor(Graphics::Cyan);
  font->print("Status: Echo", -0.98f, -1.0f);

  return true;
}
} // namespace

//
//
//
int
main(int argc, char** argv)
{
  const char* fontname = "font/SourceHanCodeJP-Normal.otf";

  auto font = GLLib::initialize("Sample", fontname, Width, Height);
  if (!font)
    return 1;

  setup();
  GLLib::bindLayer();

  // フレームループ
  for (;;)
  {
    if (GLLib::update([font]() { return onUpdate(font); }) == false)
      break;
  }

  GLLib::terminate();
  return 0;
}
