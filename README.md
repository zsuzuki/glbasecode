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
- libpng(1.6以上)
- boost(1.65以上)

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
- [imagebutton.cpp](lib/imagebutton.cpp)([.h](lib/imagebutton.h)) 画像ボタン
- [label.cpp](lib/label.cpp)([.h](lib/label.h)) 文字ラベル
- [notification.cpp](lib/notification.cpp)([.h](lib/notification.h)) 通知表示
- [primitive2d.cpp](lib/primitive2d.cpp)([.h](lib/primitive2d.h)) プリミティブ描画
- [pulldown.cpp](lib/pulldown.cpp)([.h](lib/pulldown.h)) プルダウンメニュー
- [scrollbox.cpp](lib/scrollbox.cpp)([.h](lib/scrollbox.h)) スクロールボックス
- [sheet.cpp](lib/sheet.cpp)([.h](lib/sheet.h)) 下敷きになる矩形描画
- [slidebar.cpp](lib/slidebar.cpp)([.h](lib/slidebar.h)) スライドバー
- [text.cpp](lib/text.cpp)([.h](lib/text.h)) テキスト入力
- [textbox.cpp](lib/textbox.cpp)([.h](lib/textbox.h)) テキスト入力(パーツ)
- [textbutton.cpp](lib/textbutton.cpp)([.h](lib/textbutton.h)) テキストボタン
- [texture2d.cpp](lib/texture2d.cpp)([.h](lib/texture2d.h)) テクスチャ描画

# クラス

かなり機能が増えたので改装予定。

## Graphics
glfwの機能を下地とした、グラフィック・システム機能。

## Font
FreeType2を使用したフォント描画機能。

## Primitive(2D)
矩形・ライン・円などの基本的な図形描画機能。

## Texture
テクスチャ読み込み・描画機能。現在はpng形式のみサポート。

## Label
文字列を表示する。

## Check Box
チェックボックス。

## Text Button
文字列を使ったボタン。

## Image Button
テクスチャを使用した画像ボタン。

## Pulldown
プルダウンによる選択リスト。

## Draw Box
階層化したパーツを指定領域内に描画する。シンプルなスクロールボックス。

## Scroll Box
階層化したパーツを指定領域内に描画する。

## Text Box
文字列入力。

## Slide Bar
スライダー。数値を設定する。LabelやText Boxを連携できる。

## Sheet
ただの下敷きとなる矩形を描画する。

## Dialog
ダイアログを表示。OKのみと・キャンセル付きを選択できる。

## Notification
通知メッセージを表示する。

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
