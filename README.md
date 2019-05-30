# 概要

GLFW3を使用して、最低限、フォントやプリミティブを自前で描画できるようにするためのひな形。

# ビルド

Windows/Macでの動作テスト。

## 必要ライブラリ

windows版はvcpkgで必要パッケージをインストールした。

- opengl
- glfw3
- freetype2
- glew(windowでは必要)


# クラス

## Graphics

現状ではOpenGL(glfw3)のラッパーであり、基本的なループのための機能を提供。
初期化と終了の他、現状では

- キー入力コールバック
- ファイルのドラッグアンドドロップのコールバック
- ウィンドウサイズの取得

程度の機能がある。

```c++
#include "gl.h"

int
main()
{
    if (!Graphics::initialize())
        return 1; // エラー

    while (auto window = Graphics::setupFrame())
    {
        Graphics::cleanupFrame();
    }

    Graphics::terminate();
}
```

## FontDraw

FreeType2を使用してフォントの描画を行う。

```c++
#include "font.h"

int
main()
{
    FontDraw::initialize();

    auto font = FontDraw::create("font/SourceHanCodeJP-Normal.otf");
    while (auto window = Graphics::setupFrame())
    {
        font->setColor(0.0f, 1.0f, 0.0f);
        font->print("Hello,World", 0.0f, 0.0f);

        FontDraw::render(window);
        Graphics::cleanupFrame();
    }
    FontDraw::terminate();
}
```

### initialize()/terminate()
初期化と終了処理。

### render()
Widget(後述)の`print()`によってリクエストされた文字列を描画する。
このメソッドが呼ばれないと、`Widget::print()`で設定された文字列は表示されない。

### class Widget
フォント1つの管理単位。

| 関数                                      | 機能               |
| ----------------------------------------- | ------------------ |
| setColor(float r,float g,float b,float a) | 色の設定           |
| setSize(float width,float heigth)         | フォントサイズ指定 |
| print(const char* msg,float x,float y)    | 文字列描画         |


## Primitive2D

シンプルな2Dプリミティブの描画。

```c++
#include "primitive2d.h"

int
main()
{
    Primitive2D::initialize();

    auto font = FontDraw::create("font/SourceHanCodeJP-Normal.otf");
    while (auto window = Graphics::setupFrame())
    {
        Primitive2D::setup(window);
        Primitive2D::cleanup();
        Graphics::cleanupFrame();
    }
    Primitive2D::terminate();
}
```

## initialize()/terminate()
初期化と終了処理。

## Vertex/VertexList
頂点データ(Vertex)と頂点データ配列(VertexList)。
```c++
struct Vertex {
    float x, y;
    float r, g, b, a;
};
using VertexList = std::vector<Vertex>;
```

## 描画メソッド

| 関数                                                       | 機能         |
| ---------------------------------------------------------- | ------------ |
| drawLines(const VertexList& vl,float width)                | ラインの描画 |
| drawQuads(const VertexList& vl)                            | 四角形の描画 |
| drawTriangles(const VertexList& vl)                        | 三角形の描画 |
| drawCircle(const Vertex& vl,float rad,int num,float width) | 円の描画     |

## TextButton

シンプルなボタンサポート。

```c++
#include "textbutton.h"

int
main()
{
    auto font = FontDraw::create("font/SourceHanCodeJP-Normal.otf");
    TextButton::initialize(font);

    TextButton::setButton("Hello",100,100,[]() { std::cout << "Push Button" << std::endl; });

    while (auto window = Graphics::setupFrame())
    {
        Primitive2D::setup(window);

        TextButton::update();

        Primitive2D::cleanup();
        FontDraw::render(window);
        Graphics::cleanupFrame();
    }
}

```

## TextInput

テキスト入力。

```c++
#include "text.h"

TextInput::Buffer text_buffer;
bool to_input = false;
bool end_input = false;

int
main()
{
  if (!Graphics::initialize("Sample", w, h))
    return 1;

  TextInput::setBuffer(text_buffer, "Hello");
  while ()
  {
      if (TextInput::onInput())
      {
          if (end_input)
          {
            TextInput::finish();              
            end_input = false;
            std::cout << TextInput.get() << std::endl;
          }
      }
      else if (to_input)
      {
        TextInput::start(text_buffer, 20);
        to_input = false;
      }
  }
}
```

# 参考

フォントの描画は以下を参考に。
https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_01

linmath.hは以下から取得。
https://github.com/datenwolf/linmath.h

文字コード変換は以下のサイトから取得。
https://qiita.com/benikabocha/items/e943deb299d0f816f161

床井研究室のページ。
http://marina.sys.wakayama-u.ac.jp/~tokoi/

# ライセンス

本プログラムは表示フォントに「源ノ角ゴシック Code JP」(https://github.com/adobe-fonts/source-han-code-jp) を使用しています。
Licensed under SIL Open Font License 1.1 (http://scripts.sil.org/OFL)
