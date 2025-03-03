## 4DSVMulti
OpenCVを使用した4次元ストリートビュー用のビューアーです.1つの全方位画像・動画を見るだけのプログラムもあります.
このブランチは作ったプログラムの全部入りです.Mainブランチで追加したような設定ファイル読み込み機能などはありません。

## ビルドで作られる各実行ファイルの使い道
```bash
4DSV          : 通常の4次元ストリートビュー用（ウィンドウ1枚）
4DSVVisChange : 3次元のカメラ切り替えに加えて可視化手法の切り替えも行う
OmniImage     : 1つの全方位画像を見る
OmniVideo     : 1つの全方位動画を見る
```
## 準備
C++環境とOpenCV, CMakeが必要です. Windowsの場合はC++のコンパイルにVisual Studioを使います.

### Macの場合
homebrewで
```bash
brew install opencv
brew install cmake
```
でインストールできます
### Ubuntu(Linux)の場合
```bash
sudo apt install libopencv-dev
sudo apt install cmake
```
でインストールできます
### Windowsの場合 
- Visual Studioを入れる場合はC++が使えるように「C++によるデスクトップ開発」にチェックを入れてインストールしてください.   
- OpenCVは https://opencv.org/releases/ からダウンロードして、実行してください.  
実行するとパスを聞かれるので、そのままにするか適当に決めるかして'Extract'を押して、opencvディレクトリを作ってください.  

次に、cmakeがOpenCVを認識できるようにするために、環境変数に変数OpenCV_DIRを登録します.  
例えばopencvがC:\Users\aaa\にあるなら

|変数|値|
|:-------|:----|
|OpenCV_DIR |C:\Users/aaa\opencv\build|

また、変数PATHにbinディレクトリまでのパスも追加します.
|変数|値|
|:-------|:----|
|PATH |%OpenCV_DIR%\x64\vc16\bin|

環境変数の登録が終わったらパソコンを再起動してください


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

#実行ファイルを作る(Linux,Macの場合はmakeコマンドでも作れる)
cmake --build . --config Release
```
実行ファイルと同じ名前のディレクトリがsrc以下にあります. 中には使い方を記載したREADME.mdがあるので必要に応じて参照してください.