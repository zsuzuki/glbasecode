#include "gl.h"
#include <bitset>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>

namespace Graphics
{
namespace
{
//
GLFWwindow*    window             = nullptr;
DropCallback   drop_callback      = nullptr;
KeyCallback    text_key_callback  = nullptr;
TextCallback   text_char_callback = nullptr;
ScrollCallback sbox_scr_callback  = nullptr;
KeyCallback    sbox_key_callback  = nullptr;
WindowSize     window_size{};
Locate         mouse_pos{};
Locate         mouse_pos_pd{};
Vector         mouse_scroll{};
float          xscale = 1.0f;
float          yscale = 1.0f;
WindowSize     max_size{};
Locate         base_pos{};
WindowSize     base_size{};
bool           now_fullscreen = false;
bool           enable_event   = true;
bool           pulldown_mode  = false;

// キーコードからintへの変換
int
chgCode2Num(Key::Code c)
{
  return static_cast<int>(c);
}
// サポートキーコード変換マップ
std::map<int, Key::Code> keycode_map = {
    {GLFW_KEY_UP, Key::Code::Up},
    {GLFW_KEY_DOWN, Key::Code::Down},
    {GLFW_KEY_RIGHT, Key::Code::Right},
    {GLFW_KEY_LEFT, Key::Code::Left},
    {GLFW_KEY_ENTER, Key::Code::Enter},
    {GLFW_KEY_TAB, Key::Code::Tab},
    {GLFW_KEY_BACKSPACE, Key::Code::BackSpace},
    {GLFW_KEY_DELETE, Key::Code::Delete},
    {GLFW_KEY_PAGE_UP, Key::Code::PageUp},
    {GLFW_KEY_PAGE_DOWN, Key::Code::PageDown},
    {GLFW_KEY_HOME, Key::Code::Home},
    {GLFW_KEY_END, Key::Code::End},
    {GLFW_KEY_ESCAPE, Key::Code::Esc},
    {GLFW_KEY_INSERT, Key::Code::Insert},
    {GLFW_KEY_SPACE, Key::Code::Space},
};

// メタキー入力管理クラス:実装
class KeyInputImpl : public Key::Input
{
  std::bitset<32> code;
  Key::Code       repeat;
  bool            repeat_on;
  bool            on_shift;
  bool            on_ctrl;
  bool            on_alt;

public:
  ~KeyInputImpl() override = default;

  bool      onKey(Key::Code c) const override { return code[chgCode2Num(c)]; }
  Key::Code getRepeat() const override
  {
    return repeat_on ? repeat : Key::Code::None;
  }
  bool onShift() const override { return on_shift; }
  bool onCtrl() const override { return on_ctrl; }
  bool onAlt() const override { return on_alt; }

  void clear()
  {
    code     = 0;
    repeat   = Key::Code::None;
    on_shift = false;
    on_ctrl  = false;
    on_alt   = false;
  }

  void update() { repeat_on = false; }

  // キー設定(サポートしている物のみ)
  void set(Key::Code c, int action)
  {
    switch (action)
    {
    case GLFW_REPEAT:
      repeat    = c;
      repeat_on = true;
      break;
    case GLFW_PRESS:
      code[chgCode2Num(c)] = true;
      repeat               = c;
      repeat_on            = true;
      break;
    case GLFW_RELEASE:
      code[chgCode2Num(c)] = false;
      if (repeat == c)
        repeat = Key::Code::None;
      break;
    default:
      break;
    }
  }
  // コールバックでの更新
  void update(int key, int scancode, int action, int mods)
  {
    auto k = keycode_map.find(key);
    if (k != keycode_map.end())
    {
      set(k->second, action);
    }
    on_shift = mods == GLFW_MOD_SHIFT;
    on_alt   = mods == GLFW_MOD_ALT;
    on_ctrl  = mods == GLFW_MOD_CONTROL;
  }
};
KeyInputImpl keyinput;

std::list<ClickCallback> click_callback;
using ClickAct = ClickCallback::Action;

OffEventCallback off_event_callback;

void
error_callback(int error, const char* description)
{
  std::cerr << "Error: " << description << std::endl;
}

// キー入力コールバックの大元
void
key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (enable_event)
  {
    // 通常のキー入力イベント
    if (text_key_callback)
      text_key_callback(key, scancode, action, mods);
    if (sbox_key_callback)
      sbox_key_callback(key, scancode, action, mods);
    if (key == GLFW_KEY_ENTER && action != GLFW_REPEAT)
    {
      auto act = action == GLFW_PRESS ? ClickAct::Press : ClickAct::Release;
      for (auto& fn : click_callback)
        if (fn.use_enter)
          fn.func(act, true);
    }
    keyinput.update(key, scancode, action, mods);
  }
  else if (off_event_callback.func)
  {
    // イベント発行が禁止されている場合の解除用イベント
    bool cancel_di = false;
    if (key == GLFW_KEY_ENTER)
      cancel_di = off_event_callback.func(OffEventCallback::Action::EnterKey);
    else if (key == GLFW_KEY_ESCAPE)
      cancel_di = off_event_callback.func(OffEventCallback::Action::EscapeKey);
    if (cancel_di)
      enableEvent();
  }
}

// ドラッグ＆ドロップコールバックの大元
void
dragdrop_callback(GLFWwindow* window, int num, const char** paths)
{
  if (enable_event == false)
    return;
  if (drop_callback)
    drop_callback(num, paths);
}

// マウスボタン
void
mousebutton_callback(GLFWwindow* window, int btn, int action, int mods)
{
  if (enable_event)
  {
    if (btn == GLFW_MOUSE_BUTTON_LEFT)
    {
      auto act = action == GLFW_PRESS ? ClickAct::Press : ClickAct::Release;
      for (auto& fn : click_callback)
        fn.func(act, false);
    }
  }
  else if (off_event_callback.func)
  {
    // イベント発行が禁止されている場合の解除用イベント
    if (btn == GLFW_MOUSE_BUTTON_LEFT)
    {
      if (off_event_callback.func(OffEventCallback::Action::Click))
        enableEvent();
    }
  }
}

// 文字入力コールバック
void
textinput_callback(GLFWwindow* window, unsigned int codepoint)
{
  if (enable_event == false)
    return;
  if (text_char_callback)
    text_char_callback(codepoint);
}

// スクロール
void
scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  if (enable_event == false)
    return;
  if (sbox_scr_callback)
    sbox_scr_callback(xoffset, yoffset);
  mouse_scroll.x = xoffset;
  mouse_scroll.y = yoffset;
}

} // namespace

//
KeyInput&
getKeyInput()
{
  return keyinput;
}

//
bool
initialize(const char* appname, int w, int h)
{
  glfwSetErrorCallback(error_callback);

  // GLFW初期化
  if (glfwInit() == GL_FALSE)
  {
    std::cerr << "glfwInit(): error" << std::endl;
    return false;
  }

  // ウィンドウ生成
  window = glfwCreateWindow(w, h, appname, nullptr, nullptr);
  if (!window)
  {
    glfwTerminate();
    return false;
  }

  // バージョン3.2指定
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

  // コンテキストの作成
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

#if GLFW_VERSION_MAJOR > 3 || \
    (GLFW_VERSION_MAJOR == 3 && GLFW_VERSION_MINOR >= 3)
  glfwGetWindowContentScale(window, &xscale, &yscale);
#endif

  // モニタの最大解像度を取得
  int   count;
  auto  monitor   = glfwGetPrimaryMonitor();
  auto* modes     = glfwGetVideoModes(monitor, &count);
  max_size.width  = 0;
  max_size.height = 0;
  for (int i = 0; i < count; i++)
  {
    auto& md = modes[i];
    if (max_size.width <= md.width)
    {
      max_size.width = md.width;
      if (max_size.height < md.height)
        max_size.height = md.height;
    }
  }
  // std::cout << max_size.width << "x" << max_size.height << std::endl;

#if defined(_MSC_VER)
  if (glewInit() != GLEW_OK)
  {
    std::cerr << "glewInit(): error" << std::endl;
    return false;
  }
#endif

  glfwSetKeyCallback(window, key_callback);
  glfwSetDropCallback(window, dragdrop_callback);
  glfwSetMouseButtonCallback(window, mousebutton_callback);
  glfwSetCharCallback(window, textinput_callback);
  glfwSetScrollCallback(window, scroll_callback);

  return true;
}

// 終了処理
void
terminate()
{
  glfwTerminate();
}

// フルスクリーン
void
switchFullScreen()
{
  auto  monitor = glfwGetPrimaryMonitor();
  auto* mode    = glfwGetVideoMode(monitor);
  if (now_fullscreen)
  {
    glfwSetWindowMonitor(window, nullptr, 0, 0, base_size.width / xscale,
                         base_size.height / yscale, mode->refreshRate);
    glfwSetWindowPos(window, base_pos.x, base_pos.y);
    window_size = base_size;
  }
  else
  {
    int xpos, ypos;
    glfwGetWindowPos(window, &xpos, &ypos);
    base_pos.x = xpos;
    base_pos.y = ypos;
    base_size  = window_size;
    glfwSetWindowMonitor(window, monitor, 0, 0, max_size.width, max_size.height,
                         mode->refreshRate);
  }
  now_fullscreen = !now_fullscreen;
}

// 終了
void
finish()
{
  glfwSetWindowShouldClose(window, GLFW_TRUE);
}

//
void
setDropCallback(DropCallback cb)
{
  drop_callback = cb;
}

//
void
setClickCallback(ClickCallback cb)
{
  click_callback.push_back(cb);
}

//
void
setTextInputCallback(KeyCallback kcb, TextCallback tcb)
{
  text_key_callback  = kcb;
  text_char_callback = tcb;
}

//
void
setScrollBoxFunction(ScrollCallback scb, KeyCallback kcb)
{
  sbox_scr_callback = scb;
  sbox_key_callback = kcb;
}

//
Locate
calcLocate(double x, double y, bool asp)
{
  Locate ret;
  ret.x = -1.0 + x * 2.0 / window_size.width;
  ret.y = 1.0 - y * 2.0 / window_size.height;
  if (asp)
  {
    ret.x *= window_size.width / window_size.height;
  }
  return ret;
}

//
GLFWwindow*
setupFrame()
{
  if (glfwWindowShouldClose(window) == GL_TRUE)
    return nullptr;

  int w, h;
  glfwGetFramebufferSize(window, &w, &h);
  window_size.width  = w;
  window_size.height = h;

  if (pulldown_mode)
  {
    glfwGetCursorPos(window, &mouse_pos_pd.x, &mouse_pos_pd.y);
    mouse_pos_pd.x *= xscale;
    mouse_pos_pd.y *= yscale;
  }
  else
  {
    glfwGetCursorPos(window, &mouse_pos.x, &mouse_pos.y);
    mouse_pos.x *= xscale;
    mouse_pos.y *= yscale;
  }

  glViewport(0, 0, w, h);
  glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  return window;
}

//
void
cleanupFrame()
{
  keyinput.update();

  // ダブルバッファのスワップ
  glfwSwapBuffers(window);
  glfwPollEvents();
}

//
void
setWindowSize(WindowSize ws)
{
  glfwSetWindowSize(window, ws.width / xscale, ws.height / xscale);
}

//
WindowSize
getWindowSize()
{
  return window_size;
}

//
Locate
getMousePosition()
{
  return mouse_pos;
}

//
void
setClipboardString(const char* buff)
{
  glfwSetClipboardString(window, buff);
}

//
const char*
getClipboardString()
{
  return glfwGetClipboardString(window);
}

//
void
enableScissor(double x, double y, double w, double h)
{
  glEnable(GL_SCISSOR_TEST);
  glScissor(x + 1, window_size.height - y - h + 1, w - 1, h - 1);
}
void
disableScissor()
{
  glDisable(GL_SCISSOR_TEST);
}

//
void
enableEvent()
{
  off_event_callback = {};
  enable_event       = true;
}

void
disableEvent(OffEventCallback ofc)
{
  off_event_callback = ofc;
  enable_event       = false;
}

bool
isEnabledEvent()
{
  return enable_event;
}

//
void
openPulldown()
{
  pulldown_mode = true;
}

void
closePulldown()
{
  pulldown_mode = false;
}

Locate
getPulldownCursor()
{
  return mouse_pos_pd;
}

//
Vector
getScroll()
{
  return mouse_scroll;
}

} // namespace Graphics
