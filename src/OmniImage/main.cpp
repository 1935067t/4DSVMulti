#include <sstream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <cmath>

#include "../common/axis.hpp"
#include "../common/image.hpp"
#include "../common/setting.hpp"


//スクリーンサイズ
int scWidth = 600;
int scHeight = 600;

//キーボードで回転させるときの回転の大きさ
const float rotationAngle = 0.05f;

//マウス回転用変数
cv::Point2i previousMousePos, currentMousePos , diffMousePos;
bool shouldMouseRotation = false;
float rotationSpeedXY = 0.000004f;//マウス座標の変化に定数をかけて回転の大きさを作る
float rotationSpeedZ = 0.000016f;

Axis axis;
Image image;
Setting setting;

void OperateByKey(char key)
{
    switch (key)
    {
    //回転操作
    case 'a'://左へ
        axis.Rotate(0,-rotationAngle,0);break;
    case 'd'://右へ
        axis.Rotate(0,rotationAngle,0);break;
    case 'w'://上へ
        axis.Rotate(-rotationAngle,0,0);break;
    case 's'://下へ
        axis.Rotate(rotationAngle,0,0);break;
    case 'q'://画面の奥を軸に左回転
        axis.Rotate(0,0,-rotationAngle);break;
    case 'e'://画面の奥を軸に右回転
        axis.Rotate(0,0,rotationAngle);break;

    //ズームイン・ズームアウト
    case '+':
        image.Zoom(-5.0f);break;
    case '-':
        image.Zoom(+5.0f);break;

    //視線方向をリセット
    case 'r':
        axis.Reset();break;

    //設定ウィンドウを開く・閉じて値を反映する
    case 'g':
    if(setting.isOpened() == false){
        setting.OpenWindow();
    }
    else{
        int framerate;
        setting.CloseWindow();
        std::tie(rotationSpeedXY,rotationSpeedZ,std::ignore) = setting.GetTrackbarValues();
    }
    break;
    
    default:
        break;
    }
}
void MouseCallback(int event, int x, int y, int flags, void *userdata)
{
    //回転操作開始
    if (event == cv::EVENT_LBUTTONDOWN) {
        previousMousePos.x = x - scWidth / 2;
        previousMousePos.y = y - scHeight / 2;
        shouldMouseRotation = true;
        return;
    }

    //回転操作終了
    if( event == cv::EVENT_LBUTTONUP){
        shouldMouseRotation = false;
        return;
    }

    //画面中央から離れるほど画面の奥方向に対する回転を強くする
    //x軸から離れるほど左右方向の視線変更を小さくする
    //y軸から離れるほど上下方向の視線変更を小さくする
    if(event == cv::EVENT_MOUSEMOVE){
        if(shouldMouseRotation){
            currentMousePos.x = x - scWidth / 2;
            currentMousePos.y = y - scHeight / 2;
            diffMousePos = currentMousePos - previousMousePos;

            float roll = -diffMousePos.y * (scWidth - std::abs(currentMousePos.x) * 2) * rotationSpeedXY;
            float pitch = -diffMousePos.x * (scHeight - std::abs(currentMousePos.y) * 2) * rotationSpeedXY;
            // float roll = -diffMousePos.y * (scWidth - std::abs(currentMousePos.x) * 2) / (float)(scWidth * scWidth);
            // float pitch = -diffMousePos.x * (scHeight - std::abs(currentMousePos.y) * 2) / (float)(scHeight * scHeight);
            float yaw = previousMousePos.cross(currentMousePos) * rotationSpeedZ;

            axis.Rotate(roll,pitch,yaw);
            image.MakeDstimg(axis.x, axis.y, axis.z);
            cv::imshow("dst",image.dst);

            previousMousePos.x = currentMousePos.x;
            previousMousePos.y = currentMousePos.y;
        }
        return;
    }

    //マウスのホイールを動かすとズームイン、ズームアウトする
    //Windows版のみ？
    if(event == cv::EVENT_MOUSEWHEEL){
        if(flags > 0){
            image.Zoom(-5.0f);
        }
        else{
            image.Zoom(+5.0f);
        }
        image.MakeDstimg(axis.x, axis.y, axis.z);
        cv::imshow("dst",image.dst);
    }
}

int main(int argc, char **argv) {
    if(argc == 1){
        std::cout << "please input filename";
        exit(1);
    }
    image.src = cv::imread(argv[1]);

    //ウィンドウサイズ指定
    if(argc == 4){
        scWidth = std::stoi(argv[2]);
        scHeight = std::stoi(argv[3]);
    }

    //マウスによる回転の速度設定
    rotationSpeedXY = 1.0f / (scWidth * scHeight);
    if(scWidth > scHeight){
        rotationSpeedZ = 1.0f /((scWidth / 2) * (scWidth / 2));
    }
    else{
        rotationSpeedZ = 1.0f / ((scHeight / 2) * (scHeight / 2));
    }

    image.SetDstMat(cv::Size(scWidth, scHeight));
    image.MakeDstimg(axis.x, axis.y, axis.z);
    setting.SetInitialValues(rotationSpeedXY, rotationSpeedZ, 1);
    // DrawAxis(dstimg, xAxis, yAxis, zAxis, cv::Point(250, 250));
    cv::namedWindow("dst");
    cv::setMouseCallback("dst",MouseCallback);
    cv::imshow("dst",image.dst);

    std::cout << "image  size: " << image.src.size << std::endl;
    std::cout << "window size: " << scWidth << " x " << scHeight << std::endl; 

    while(true){
        int keyI = cv::waitKey(0);
        char keyC = (char)keyI;

        //ウィンドウが閉じられた時(-1),またはescape(27)が押されたとき
        if(keyI == -1 || keyI == 27){
            break;
        }

        OperateByKey(keyC);

        image.MakeDstimg(axis.x, axis.y, axis.z);
        // DrawAxis(dstimg, xAxis, yAxis, zAxis, cv::Point(250, 250));
        cv::imshow("dst",image.dst);
    }
    return 0;
}