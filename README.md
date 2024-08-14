## 準備
OpenCV, CMakeが必要です.  
macの場合はhomebrewで
```bash
brew install opencv
brew install cmake
```
とすればインストールできます.

## 使い方
### 1.　ビルドする
```bash
#任意のディレクトリにクローンする
git clone https://github.com/1935067t/4DSVMulti.git

#4DSVMultiに移動. 移動先にはCMakeList.txtがあるはず
cd 4DSVMulti

#buildディレクトリを作る
mkdir build

#buildディレクトリに移動
cd build

#cmakeコマンドでMakeFileを作る
cmake ..

#cmakeが成功したらmakeで実行ファイルを作る. mainというバイナリファイルが作られる
make
```

### 2. コンフィグファイルを作る
動画ファイルの場所やカメラ群の配置情報を知らせるためのconfigfileを作ります.configには以下の内容を記載してください
```text
・mainウィンドウで表示する動画群のディレクトリパス
・動画の拡張子
・カメラを何×何×何で配置したか(dimension)
・ウィンドウに表示する最初の動画位置(position)
・１秒に何フレーム動画を進めるか(速すぎると計算が追いつかないと思います)
・subウィンドウで表示する動画群のディレクトリパス
```
例としてexample.4dsvを用意しているので参照してください

### 3. 実行&使い方
実行時にコンフィグファイルを読み込みます
```bash
./main example.4dsv
```
動画の再生・停止、視線方向の変更などはキーボード操作で行います.操作方法は以下の通りです

```bash
#回転操作
A , D     :　左右方向に視線を移動する

W , S     :　上下方向に視線を移動する

E , Q     :　時計回り、反時計回りに画面を回転させる

#動画操作
Space     :　動画の再生・停止切り替え

Enter     :　動画を１フレーム進める

Backspace,
Delete    :　動画を１フレーム戻す

0         :  動画を最初のフレームに戻す

#4DSV操作(動画の切り替え)
H , L     :　X方向に-1,か +1 positionを変える

J , K     :　Y方向に-1,か +1 positionを変える

"," , "." :　Z方向に-1,か +1 positionを変える

#それ以外
Escape    :　アプリ終了

V         :　UIの表示/非表示切り替え

"+" , "-" :  ズームイン/ズームアウト
```
