## 4DSVMulti
OpenCVを使用した4次元ストリートビュー用のビュワープログラムです.1つの全方位画像・動画を見るだけのプログラムもあります.

## ビルドで作られる各実行ファイルの使い道
```bash
4DSV          : 通常の4次元ストリートビュー用（ウィンドウ1枚）
4DSVVisChange : 3次元のカメラ切り替えに加えて可視化手法の切り替えも行う
4DSVMulti     : 4次元ストリートビューでウィンドウを2枚出す時のもの
OmniImage     : 1つの全方位画像を見る
OmniVideo     : 1つの全方位動画を見る
```
## 準備
OpenCV, CMakeが必要です.  
macの場合はhomebrewで
```bash
brew install opencv
brew install cmake
```
ubuntu(linux)の場合は
```bash
sudo apt install libopencv-dev
sudo apt install cmake
```
とすればインストールできます.

## ビルドのやり方
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

#cmakeが成功したらmakeで実行ファイルを作る. 実行ファイルが5つ作られる
make
```
実行ファイルと同じ名前のディレクトリがsrc以下にあります.中には使い方を記載したREADME.mdがあるので必要に応じて参照してください.