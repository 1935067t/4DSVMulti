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

### 2. 実行&使い方
実行時に引数として読み込むファイルやウィンドウサイズを指定します
```bash
#ウィンドウサイズを指定しない場合
./main filename

#ウィンドウサイズを指定する場合
./main filename window_width window_height
```
回転操作などはキーボード操作で行います.操作方法は以下の通りです

```bash
#回転操作
A , D     :　左右方向に視線を移動する

W , S     :　上下方向に視線を移動する

E , Q     :　時計回り、反時計回りに画面を回転させる

R         :  回転のリセット

#それ以外
Escape    :　アプリ終了

"+" , "-" :  ズームイン/ズームアウト
```
