# 概要

GLFW3を使用してシンプルなUIを構築し、簡単なグラフィックを扱えるツールを作るための、ベースとなるコード。

# ビルド

Windows/Macでの動作テストしている。linuxでも多分動くはず。

## 必要ライブラリ

windows版はvcpkgで必要パッケージをインストールした。
macはbrew、もしくは自前ビルドで用意。

- opengl
- glfw3
- freetype2
- glew(windowでは必要)

## ビルド手順

以下のコマンドを実行し、出来上がったプロジェクトファイルをVisual Studio/Xcodeで開いて使用する。
```shell
> mkdir build
> cd build
> cmake ..
```
もしくは[ninja](https://ninja-build.org/)を使用する場合。
```shell
> cmake -G Ninja ..
> ninja
```

# ファイル構成

基本的には"gllib.h"をインクルードすれば全機能にアクセスできる。

- [gl.cpp](lib/gl.cpp)([.h](lib/gl.h)) gl,glfwへのインターフェース
- [gllib.h](lib/gllib.h) アプリケーション用ヘッダ
- [bb.h](lib/bb.h) バウンディングボックス
- [layer.h](lib/layer.h) レイヤー切り替え制御
- [linmath.h](lib/linmath.h)ベクトル演算
- [parts.h](lib/parts.h) 各パーツの基底クラス定義
- [checkbox.cpp](lib/checkbox.cpp)([.h](lib/checkbox.h)) チェックボックス
- [codeconv.h](lib/codeconv.h) 文字コード変換
- [dialog.cpp](lib/dialog.cpp)([.h](lib/dialog.h)) ダイアログ表示
- [drawbox.cpp](lib/drawbox.cpp)([.h](lib/drawbox.h)) スクロール対応描画領域
- [exec.cpp](lib/exec.cpp)([.h](lib/exec.h)) 子プロセス起動
- [font.cpp](lib/font.cpp)([.h](lib/font.h)) フォント描画
- [label.cpp](lib/label.cpp)([.h](lib/label.h)) 文字ラベル
- [primitive2d.cpp](lib/primitive2d.cpp)([.h](lib/primitive2d.h)) プリミティブ描画
- [pulldown.cpp](lib/pulldown.cpp)([.h](lib/pulldown.h)) プルダウンメニュー
- [scrollbox.cpp](lib/scrollbox.cpp)([.h](lib/scrollbox.h)) スクロールボックス
- [text.cpp](lib/text.cpp)([.h](lib/text.h)) テキスト入力
- [textbox.cpp](lib/textbox.cpp)([.h](lib/textbox.h)) テキスト入力(パーツ)
- [textbutton.cpp](lib/textbutton.cpp)([.h](lib/textbutton.h)) テキストボタン

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

アイコンのPNGデータについては「MATERIAL DESIGN」サイトのものを使用しています。
(https://material.io/resources/icons/?style=baseline)
このアイコンは[Apache2.0ライセンス](http://www.apache.org/licenses/LICENSE-2.0)で配布されています。
