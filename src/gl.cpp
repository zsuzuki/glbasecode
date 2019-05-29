#include "gl.h"
#include <iostream>

namespace Graphics
{
namespace
{
//
GLFWwindow*  window        = nullptr;
KeyCallback  key_callback  = nullptr;
DropCallback drop_callback = nullptr;
WindowSize   window_size{};

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
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

  // コンテキストの作成
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

#if defined(_MSC_VER)
  if (glewInit() != GLEW_OK)
  {
    std::cerr << "glewInit(): error" << std::endl;
    return false;
  }
#endif

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
  glfwSetKeyCallback(
      window, [](auto window, int key, int scancode, int action, int mods) {
        if (key_callback)
          key_callback(key, scancode, action, mods);
      });
}
//
void
setDropCallback(DropCallback cb)
{
  drop_callback = cb;
  glfwSetDropCallback(window, [](auto window, int num, const char** paths) {
    if (drop_callback)
      drop_callback(num, paths);
  });
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

} // namespace Graphics
