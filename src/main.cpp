#include <exec.h>
#include <gllib.h>
#include <iostream>
#include <list>

namespace
{
int  Width    = 1600;
int  Height   = 1200;
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

  auto y   = 300.0;
  auto btn = TextButton::setButton("Disp Primitive", 200, y,
                                   []() { DispPrim = !DispPrim; });
  y += btn->getHeight() + 80.0;
  auto tb = TextBox::create("", 200, y, 400);
  tb->setMaxLength(16);
  tb->setPlaceHolder("filename");
  // y += tb->getHeight() * 80.0;
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

  static bool stick_x = false, stick_y = false;
  auto        chx = 350.0;
  auto        chy = 200.0;
  auto        ch  = CheckBox::create("Sticky X", chx, chy, stick_x);
  ch->setChanged([SBox](bool s) {
    stick_x = s;
    SBox->setSticky(stick_x, stick_y);
  });
  chx += ch->getWidth() + 40.0;
  ch = CheckBox::create("Sticky Y", chx, chy, stick_y);
  ch->setChanged([SBox](bool s) {
    stick_y = s;
    SBox->setSticky(stick_x, stick_y);
  });

  double by = 100.0;
  for (int i = 0; i < 20; i++)
  {
    double x = 60.0;

    auto cb = CheckBox::create("Chk", x, by, false);
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

  double y = 300;

  Pulldown::List pl{"PULLDOWN 1", "PULLDOWN 2", "PULLDOWN 3", "PULLDOWN 4",
                    "TEST 5",     "TEST 6",     "SAMPLE 7",   "SAMPLE 8"};
  auto           pd = Pulldown::create(std::move(pl), 5);
  auto           tb = TextButton::setButton("Pulldown", 200, y, []() {});
  tb->setPulldown(pd);
  pd->setSelected([tb](int idx, auto s) {
    tb->setCaption("Select " + std::to_string(idx));
  });
  pd->setChanged([tb](int idx, auto s) {
    tb->setCaption("Change " + std::to_string(idx));
  });

  Pulldown::List tp{"first", "second", "third", "forth", "fifth"};
  pd = Pulldown::create(std::move(tp), 5);

  y += tb->getHeight() + 50;
  auto tt = TextBox::create("", 200, y, 400, 70);
  tt->setMaxLength(16);
  tt->setPlaceHolder("complete field");
  tt->setPulldown(pd);

  y += tt->getHeight() + 50;
  tt = TextBox::create("", 200, y, 400, 70);
  tt->setMaxLength(16);
  tt->setPlaceHolder("normal text");
}

//
void
setup(FontDraw::WidgetPtr font)
{
  Graphics::setDropCallback(drop_file);
  Graphics::setWindowSize({(double)Width, (double)Height});

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
    auto dlg1 = Dialog::create("Welcome\nHello, World\nDialog");
    auto dlg2 = Dialog::create("OK or Cancel ?", true);
    dlg1->setOK([](bool) { std::cout << "OK Dialog1" << std::endl; });
    dlg2->setOK([](bool) { std::cout << "OK Dialog2" << std::endl; });
    dlg2->setCancel([](bool) { std::cout << "Cancel Dialog2" << std::endl; });
    auto btnx = Width - font->getSizeX() * 12.0 - 50;
    auto db =
        setButton("OK Dialog", btnx, 300, [dlg1]() { Dialog::open(dlg1); });
    setButton("With Cancel", btnx, 300 + db->getHeight() + 40,
              [dlg2]() { Dialog::open(dlg2); });
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
using DBoxList = std::initializer_list<DrawBox::BoxPtr>;
using ImgList  = std::initializer_list<Texture2D::ImagePtr>;
bool
onUpdate(FontDraw::WidgetPtr font, DBoxList dbl, ImgList imgl)
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
  font->setDepth(-0.5f);
  font->print("こんにちは、世界", -0.25f, 0.5f);
  font->setColor(Graphics::Cyan);
  font->print("Status: Echo", -0.98f, -0.98f);

  // 描画ボックス内にフォント表示
  auto dbi  = dbl.begin();
  auto dbox = *dbi;
  dbox->begin();
  for (int i = 0; i < 15; i++)
  {
    double x = dbox->getBaseX() + 20.0;
    double y = dbox->getBaseY() + i * 50.0;
    auto   l = Graphics::calcLocate(x, y);
    font->setColor(Graphics::Orange);
    char buff[64];
    std::snprintf(buff, sizeof(buff), "DrawBox PRINT SAMPLE No. %2d", i);
    font->print(buff, l.x, l.y);
  }
  dbox->end();
  dbox = *(++dbi);
  dbox->begin();
  for (int i = 0; i < 20; i++)
  {
    double x = dbox->getBaseX() + 20.0;
    double y = dbox->getBaseY() + i * 50.0;
    auto   l = Graphics::calcLocate(x, y);
    font->setColor(Graphics::Yellow);
    char buff[64];
    std::snprintf(buff, sizeof(buff), "No. %2d TEST DrawBox", i);
    font->print(buff, l.x, l.y);
  }
  dbox->end();

  auto imgi = imgl.begin();
  auto img1 = *imgi;
  Texture2D::draw(img1, 0.0, 0.0, 0.4, 0.4);

  auto img2 = *(++imgi);
  Texture2D::draw(img2, -0.5, 0.7, 0.4, 0.4);

  return true;
}
} // namespace

//
//
//
int
main(int argc, char** argv)
{
  const char* fontname = "res/SourceHanCodeJP-Normal.otf";

  auto font = GLLib::initialize("Sample", fontname, Width, Height);
  if (!font)
    return 1;

  setup(font);
  GLLib::bindLayer();

  // DrawBox
  auto dbox1 = DrawBox::create(font, 200, 700, 300, 400);
  auto dbox2 = DrawBox::create(font, 800, 700, 300, 400);
  auto dbl   = {dbox1, dbox2};
  dbox1->setDrawSize(600, 750);
  dbox2->setDrawSize(600, 750);
  dbox1->setLink(dbox2.get());

  // Image
  auto img1 = Texture2D::create("res/test.png");
  auto img2 = Texture2D::create("res/textest.png");
  auto imgl = {img1, img2};

  // フレームループ
  for (;;)
  {
    if (GLLib::update([&]() { return onUpdate(font, dbl, imgl); }) == false)
      break;
  }

  GLLib::terminate();
  return 0;
}
