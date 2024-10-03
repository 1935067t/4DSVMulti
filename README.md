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

#cmakeが成功したらmakeで実行ファイルを作る. OmniVideoViewerというバイナリファイルが作られる
make
```

### 2. 実行&使い方
実行時にファイルパスを指定します.　オプションでウィンドウサイズを指定することもできます。
```bash
./main filepath

#ウィンドウサイズを指定する場合
./main filepath window_width window_height
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

#それ以外
Escape    :　アプリ終了

V         :　UIの表示/非表示切り替え

"+" , "-" :  ズームイン/ズームアウト
```
