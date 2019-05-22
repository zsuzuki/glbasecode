#ifdef MSC_VER
#include <GL/gl.h>
#include <Windows.h>
#else
#include <OpenGL/gl.h>
#endif
#include <GLFW/glfw3.h>
#include <iostream>

#include "font.h"

namespace
{
void
error_callback(int error, const char* description)
{
  std::cerr << "Error: " << description << std::endl;
}
void
key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}
void
drop_file(GLFWwindow* window, int num, const char** paths)
{
  for (int i = 0; i < num; i++)
  {
    std::cout << paths[i] << std::endl;
  }
}
} // namespace

int
main(int argc, char** argv)
{
  glfwSetErrorCallback(error_callback);

  // GLFW初期化
  if (glfwInit() == GL_FALSE)
  {
    return 1;
  }

  // ウィンドウ生成
  const char* title  = "Sample";
  int         w      = 640;
  int         h      = 480;
  GLFWwindow* window = glfwCreateWindow(w, h, title, nullptr, nullptr);
  if (!window)
  {
    glfwTerminate();
    return 1;
  }

  // callback
  glfwSetKeyCallback(window, key_callback);
  glfwSetDropCallback(window, drop_file);

  // バージョン2.1指定
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

  // コンテキストの作成
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  extern void testInit();
  extern void testSetup();
  extern void testRender(GLFWwindow*);

  testInit();

  FontDraw::initialize();

  auto font = FontDraw::create("font/SourceHanCodeJP-Normal.otf");

  // フレームループ
  while (glfwWindowShouldClose(window) == GL_FALSE)
  {
    // バッファのクリア
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    testSetup();
    testRender(window);

    font->setColor(0.0f, 1.0f, 0.0f);
    font->print("Hello,World", 0.0f, 0.0f);
    font->setColor(0.8f, 0.8f, 1.0f);
    font->print("Status: Echo", -0.98f, -1.0f);
    font->print("Title: Top", -0.98f, 1.0f - (32.0f / 480.0f));
    FontDraw::render(window);

    // ダブルバッファのスワップ
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  FontDraw::terminate();

  // GLFWの終了処理
  glfwTerminate();

  return 0;
}
