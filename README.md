## 4DSVMulti
OpenCVを使用した4次元ストリートビュー用のビューアーです.1つの全方位画像・動画を見るだけのプログラムもあります.
このブランチは作ったプログラムの全部入りです.Mainブランチで追加したような設定ファイル読み込み機能などはありません。

## ビルドで作られる各実行ファイルの使い道
```bash
4DSV          : 通常の4次元ストリートビュー用（ウィンドウ1枚）
4DSVVisChange : 3次元のカメラ切り替えに加えて可視化手法の切り替えも行う
4DSVResChange : 解像度切り替え用
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

#cmakeコマンドでMakeFileを作る(Windowsの場合はVisual Studioソリューションが作られる)
cmake ..

#実行ファイルを作る
cmake --build . --config Release

#Linux,Macの場合はmakeコマンドでも実行ファイルを作ることが出来る
```
実行ファイルと同じ名前のディレクトリがsrc以下にあります. 中には使い方を記載したREADME.mdがあるので必要に応じて参照してください.