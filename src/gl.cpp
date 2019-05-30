#include "gl.h"
#include <iomanip>
#include <iostream>

namespace Graphics
{
namespace
{
//
GLFWwindow*      window             = nullptr;
KeyCallback      key_callback       = nullptr;
DropCallback     drop_callback      = nullptr;
MouseBtnCallback mbtn_callback      = nullptr;
KeyCallback      text_key_callback  = nullptr;
TextCallback     text_char_callback = nullptr;
MouseBtnCallback tbtn_callback      = nullptr;
WindowSize       window_size{};
Locate           mouse_pos{};
float            xscale = 1.0f;
float            yscale = 1.0f;

void
error_callback(int error, const char* description)
{
  std::cerr << "Error: " << description << std::endl;
}

} // namespace

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

  // バージョン2.1指定
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  // コンテキストの作成
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  glfwGetWindowContentScale(window, &xscale, &yscale);

#if defined(_MSC_VER)
  if (glewInit() != GLEW_OK)
  {
    std::cerr << "glewInit(): error" << std::endl;
    return false;
  }
#endif

  glfwSetKeyCallback(
      window, [](auto window, int key, int scancode, int action, int mods) {
        if (key_callback)
          key_callback(key, scancode, action, mods);
        if (text_key_callback)
          text_key_callback(key, scancode, action, mods);
      });
  glfwSetDropCallback(window, [](auto window, int num, const char** paths) {
    if (drop_callback)
      drop_callback(num, paths);
  });
  glfwSetMouseButtonCallback(window,
                             [](auto window, int btn, int action, int mods) {
                               if (mbtn_callback)
                                 mbtn_callback(btn, action, mods);
                               if (tbtn_callback)
                                 tbtn_callback(btn, action, mods);
                             });
  glfwSetCharCallback(window, [](auto window, unsigned int codepoint) {
    if (text_char_callback)
      text_char_callback(codepoint);
  });

  return true;
}

// 終了処理
void
terminate()
{
  glfwTerminate();
}

// 終了
void
finish()
{
  glfwSetWindowShouldClose(window, GLFW_TRUE);
}

//
void
setKeyCallback(KeyCallback cb)
{
  key_callback = cb;
}
//
void
setDropCallback(DropCallback cb)
{
  drop_callback = cb;
}

//
void
setMouseButtonCallback(MouseBtnCallback cb)
{
  mbtn_callback = cb;
}

//
void
setTextButtonCallback(MouseBtnCallback cb)
{
  tbtn_callback = cb;
}

//
void
setTextInputCallback(KeyCallback kcb, TextCallback tcb)
{
  text_key_callback  = kcb;
  text_char_callback = tcb;
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

  glfwGetCursorPos(window, &mouse_pos.x, &mouse_pos.y);
  mouse_pos.x *= xscale;
  mouse_pos.y *= yscale;

  glViewport(0, 0, w, h);
  glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  return window;
}

//
void
cleanupFrame()
{
  // ダブルバッファのスワップ
  glfwSwapBuffers(window);
  glfwPollEvents();
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

} // namespace Graphics
