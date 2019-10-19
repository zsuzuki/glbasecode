#include <cmath>
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

std::vector<Graphics::Color> color_list = {
    Graphics::White,  Graphics::Red,   Graphics::Green,   Graphics::Blue,
    Graphics::Yellow, Graphics::Cyan,  Graphics::Magenta, Graphics::Orange,
    Graphics::Gray,   Graphics::Sepia,
};

std::vector<Texture2D::Align> tex_align_list = {
    Texture2D::Align::LeftTop,     Texture2D::Align::Left,
    Texture2D::Align::LeftBottom,  Texture2D::Align::CenterTop,
    Texture2D::Align::Center,      Texture2D::Align::CenterBottom,
    Texture2D::Align::RightTop,    Texture2D::Align::Right,
    Texture2D::Align::RightBottom,
};
int    tex_align  = 4;
int    tex_color  = 0;
double tex_rotate = 0.0;
bool   tex_rot    = true;

//
void
setupMenu1()
{
  Label::create("SETTINGS", 100, 80, Graphics::Green, Graphics::Gray);
  TextButton::setButton("Return", 100, 150, []() { GLLib::bindLayer(); });

  auto x = 200.0;
  auto y = 300.0;

  // プリミティブの表示・非表示
  auto btn = TextButton::setButton("Disp Primitive", x, y,
                                   []() { DispPrim = !DispPrim; });

  // テクスチャ表示のアライン
  y += btn->getHeight() + 20.0;
  Pulldown::List pl{"Left Top",   "Left",   "Left Bottom",
                    "Center Top", "Center", "Center Bottom",
                    "Right Top",  "Right",  "Right Bottom"};

  auto pd = Pulldown::create(std::move(pl), 6);
  auto tb = TextButton::setButton("Tex Align", x, y, []() {});
  tb->setPulldown(pd);
  pd->setSelected([](int idx, auto) { tex_align = idx; });

  // テクスチャの色
  y += tb->getHeight() + 20.0;
  Pulldown::List cl = {
      "White", "Red",     "Green",  "Blue", "Yellow",
      "Cyan",  "Magenta", "Orange", "Gray", "Sepia",
  };
  pd = Pulldown::create(std::move(cl), 6);
  tb = TextButton::setButton("Tex Color", x, y, []() {});
  tb->setPulldown(pd);
  pd->setSelected([](int idx, auto) { tex_color = idx; });

  // テクスチャの回転 ON/OFF
  y += tb->getHeight() + 20.0;
  auto ch = CheckBox::create("Tex Rotate", x, y, tex_rot);
  ch->setChanged([](bool s) { tex_rot = s; });
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
  SBox->setDepth(0.5f);
  SBox->setScrollConstraint(true, false);
  SBox->setFocusBorderColor({0.0f, 1.0f, 1.0f, 1.0f});

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
    x += cb->getWidth() + 40.0;
    SBox->append(cb);

    auto ib = ImageButton::create("res/check.png", x, by, []() {});
    ib->setFocusColor(Graphics::Orange);
    SBox->append(ib);
    x += ib->getWidth() + 40.0;

    std::string l = "Item";
    l += std::to_string(i + 1);
    auto p  = TextButton::setButton(l, x, by, []() {});
    auto bg = Graphics::Black;
    bg.a    = 0.1f;
    p->setColor(TextButton::ColorType::UnFocusBG, bg);
    by += p->getHeight() + 10;
    SBox->append(p);
  }
  auto tfg = Graphics::Yellow;
  auto tbg = Graphics::Blue;
  SBox->append(Label::create("Scroll Menu", 150, 20, tfg, tbg));

  auto sh = Sheet::create(10, 10, 500, 80);
  SBox->append(sh);
  sh->setFillColor(Graphics::Red);
  sh->setBorderColor(Graphics::Yellow);
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

  y += tt->getHeight() + 50;
  tt = TextBox::create("", 200, y, 400, 70);
  tt->setMaxLength(16);
  tt->setInputStyle(Text::InputStyle::Number);
  tt->setPlaceHolder("number input");
}

//
void
setupMenu4()
{
  Label::create("PARAMETER", 100, 80, Graphics::Yellow, Graphics::Gray);
  TextButton::setButton("Return", 100, 150, []() { GLLib::bindLayer(); });

  double x = 300;
  double y = 300;

  double sx = x, sy = y, sw = 0.0, sh = 0.0;
  auto   adjsh = [&](auto w) {
    double tx = w->getX() + w->getWidth();
    if (tx > sw)
      sw = tx;
    double ty = w->getY() + w->getHeight();
    if (ty > sh)
      sh = ty;
  };

  auto sb = SlideBar::create(x, y, 400, 50);
  auto lx = x + sb->getWidth() + 20;
  auto lb = Label::create("", lx, y, Graphics::White, Graphics::ClearColor);
  lb->setSlider(sb);
  sb->setNumber(0.5);
  adjsh(sb);
  //
  y += sb->getHeight() + 20;
  sb = SlideBar::create(x, y, 500, 50);
  sb->setMinMax(-1, 20);
  sb->setStep(1);
  sb->setNumberType(SlideBar::Type::Integer);
  sb->setChanged([](auto n) { std::cout << "Slide: " << n << std::endl; });
  lb = Label::create("", x, y, Graphics::White, Graphics::ClearColor);
  lb->setSlider(sb, 0);
  adjsh(sb);
  //
  y += sb->getHeight() + 20;
  sb = SlideBar::create(x, y, 600, 50);
  sb->setNumber(1.0);
  adjsh(sb);
  //
  sx -= 20;
  sy -= 20;
  sw += 20;
  sh += 20;
  auto st = Sheet::create(sx, sy, sw - sx, sh - sy);
  st->setFillColor({0.0f, 0.2f, 0.1f, 0.8f});
  st->setBorderColor(Graphics::White);
}

//
void
setup(FontDraw::WidgetPtr font)
{
  Graphics::setDropCallback(drop_file);
  Graphics::setWindowSize({(double)Width, (double)Height});

  // top menu
  {
    // 終了ボタン
    auto ib = ImageButton::create("res/close.png", Width - 58, 10,
                                  []() { Graphics::finish(); });
    ib->setFocusColor(Graphics::Green);
    ib->setFocusIcon("res/exit_to_app.png");
    // ダイアログ
    int  dicon_id = Dialog::registIcon("res/warning.png");
    auto dlg1     = Dialog::create("Welcome\nHello, World\nDialog");
    auto dlg2     = Dialog::create("OK or Cancel ?", true);
    dlg1->setIcon(dicon_id);
    dlg1->setOK([](bool) { std::cout << "OK Dialog1" << std::endl; });
    dlg2->setOK([](bool) { std::cout << "OK Dialog2" << std::endl; });
    dlg2->setCancel([](bool) { std::cout << "Cancel Dialog2" << std::endl; });
    // ダイアログ呼び出し
    auto btnx = Width - font->getSizeX() * 12.0 - 50;
    auto btny = 300;
    auto db   = TextButton::setButton("OK Dialog", btnx, btny,
                                    [dlg1]() { Dialog::open(dlg1); });
    btny += db->getHeight() + 40;
    db = TextButton::setButton("With Cancel", btnx, btny,
                               [dlg2]() { Dialog::open(dlg2); });
    btny += db->getHeight() + 40;
    // 通知
    auto nficon_id = Notification::registIcon("res/notification_important.png");
    TextButton::setButton("notify", btnx, btny, [nficon_id]() {
      static bool cnt = true;
      auto        n   = Notification::notify(cnt ? "Hello" : "Notification");
      n->setFontColor(cnt ? Graphics::White : Graphics::Orange);
      n->setBorderColor(cnt ? Graphics::White : Graphics::Orange);
      if (!cnt)
        n->setIcon(nficon_id);
      cnt = !cnt;
    });
  }
  for (int i = 0; i < 4; i++)
  {
    int  idx = i + 1;
    auto t   = std::string("MENU") + std::to_string(idx);
    GLLib::bindLayer();
    auto mfunc = [t]() { GLLib::bindLayer(t); };
    auto btn   = TextButton::setButton(t, 150, 150 + i * 80, mfunc);
    btn->setColor(TextButton::ColorType::Border, Graphics::Cyan);
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
    case 4:
      setupMenu4();
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
    Primitive2D::setDepth(0.99f);
    Primitive2D::drawQuads(vl);
    static const Primitive2D::Vertex v = {0.0f, 0.0f, 0.5f, 1.0f, 1.0f};
    Primitive2D::drawCircle(v, 0.5f, 32, 8.0f);
  }

  // グラフィック座標系で毎フレーム描画
  font->setColor(Graphics::Green);
  font->setDepth(-0.5f);
  font->print("こんにちは、世界", -0.25f, 0.55f);
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
  auto img2 = *(++imgi);

  Texture2D::DrawSet dset;
  dset.image  = img1;
  dset.width  = 0.4;
  dset.height = 0.4 * ((double)img1->getHeight() / (double)img1->getWidth());
  dset.depth  = 0.95f;
  Texture2D::draw(dset);

  auto ws     = Graphics::getWindowSize();
  dset.image  = img2;
  dset.x      = -0.5;
  dset.y      = 0.5;
  dset.width  = img2->getWidth() * 2.0 / ws.width;
  dset.height = img2->getHeight() * 2.0 / ws.width;
  dset.rotate = tex_rotate;
  dset.align  = tex_align_list[tex_align];
  dset.color  = color_list[tex_color];
  Texture2D::draw(dset);
  tex_rotate += tex_rot ? M_PI / 180.0 : 0.0;

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
