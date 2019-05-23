# 概要

GLFW3を使用して、最低限、フォントやプリミティブを自前で描画できるようにするためのひな形。

# ビルド

Windows/Macでの動作テスト。

## 必要ライブラリ

- opengl
- glfw3
- freetype2

# クラス

## FontDraw

FreeType2を使用してフォントの描画を行う。

```c++
#include "font.h"

int
main()
{
    GLFWwindow* window = glfwCreateWindow(w, h, title, nullptr, nullptr);
    FontDraw::initialize();

    auto font = FontDraw::create("font/SourceHanCodeJP-Normal.otf");
    while (true)
    {
        font->setColor(0.0f, 1.0f, 0.0f);
        font->print("Hello,World", 0.0f, 0.0f);

        FontDraw::render(window);
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
    GLFWwindow* window = glfwCreateWindow(w, h, title, nullptr, nullptr);
    Primitive2D::initialize();

    auto font = FontDraw::create("font/SourceHanCodeJP-Normal.otf");
    while (true)
    {
        Primitive2D::setup(window);
        Primitive2D::cleanup();
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
