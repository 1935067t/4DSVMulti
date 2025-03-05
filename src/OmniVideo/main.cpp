#include <opencv2/opencv.hpp>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>

#include "../common/video.hpp"
#include "../common/slider.hpp"
#include "../common/axis.hpp"
#include "../common/image.hpp"
#include "../common/setting.hpp"

namespace fs = std::filesystem;

//デフォルトスクリーンサイズ
int scHeight = 600;
int scWidth = 600;

//マウスドラッグによる回転
cv::Point2i previousMousePos, currentMousePos , diffMousePos;
bool shouldMouseRotation = false;

//マウスによる回転の補整
float rotationSpeedXY = 0.000004f;
float rotationSpeedZ = 0.000016f;

Axis axis;
Image image;

Video video;
Slider slider;
Setting setting;

bool uiVisibility = true;

void ReadSettingfile(char *filename)
{
    std::ifstream fs(filename);
    if(!fs){
        std::cerr << "can't open setting file" << std::endl;
        exit;
    }

    std::string line;
    while(std::getline(fs,line)){
        std::stringstream ss(line);
        std::string entry;
        int r,g,b;
        ss >> entry;

        if(entry == "SIZE"){
            ss >> scWidth >> scHeight;
        }
        else if(entry == "FONT"){
            ss >> r >> g >> b;
            slider.SetFontColor(cv::Scalar(b,g,r));
        }
        else if(entry == "SLIDER"){
            ss >> r >> g >> b;
            slider.SetColor(cv::Scalar(b,g,r));
        }
        else if(entry == "SLIDERBG"){
            ss >> r >> g >> b;
            slider.SetBGColor(cv::Scalar(b,g,r));
        }
    }
}

void InitSlider()
{
    int sliderWidth = 150;
    int sliderHeight = 10;
    int sliderPaddingWidth = 20;//スライダーを画面端からどれだけ離すか
    int sliderPaddingHeight = 20;
    int sliderCollisionPadding = 10;//スライダーの操作判定を見た目より少し大きくする
    int sliderStartWidth = scWidth - sliderWidth - sliderPaddingWidth;
    int sliderStartHeight = scHeight - sliderHeight - sliderPaddingHeight;

    slider.SetShape(cv::Point2i(sliderStartWidth,sliderStartHeight),
                    cv::Size2i(sliderWidth,sliderHeight));
    slider.SetTotalCount(video.FrameCount());
}

void OperateByKey(char key)
{
    switch (key)
    {
    case 'a'://左へ
        axis.Rotate(0,-0.05f,0);break;
    case 'd'://右へ
        axis.Rotate(0,0.05f,0);break;
    case 'w'://上へ
        axis.Rotate(-0.05f,0,0);break;
    case 's'://下へ
        axis.Rotate(0.05f,0,0);break;
    case 'q'://左回転
        axis.Rotate(0,0,-0.05f);break;
    case 'e'://右回転
        axis.Rotate(0,0,0.05f);break;
    
    default:
        break;
    }
}

void OperateVideoByKeyInput(char key)
{
    switch (key)
    {
    case ' '://space
        video.SwitchPlaying();
        break;

    case 13://Enter
        video.Stop();
        video.StepForward(image.src);
        break;

    case 8:  //backspace
    case 127://delete
        video.SeekFrame(video.CurrentFrame() - 1, image.src);
        break;

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        video.SeekFrame(video.FrameCount() * (key - '0') * 0.1,image.src);
        break;

    case 'v':
        uiVisibility = !uiVisibility;
        break;

    //ズームイン、ズームアウト
    case '+':
        image.Zoom(-5.0f);
        break;

    case '-':
        image.Zoom(5.0f);
        break;

    //再生スピードなどの設定ウィンドウを開く・閉じて値を反映する
    case 'g':
        if(setting.isOpened() == false){
            setting.OpenWindow();
            video.Stop();
        }
        else{
            int framerate;
            setting.CloseWindow();
            std::tie(rotationSpeedXY,rotationSpeedZ,framerate) = setting.GetTrackbarValues();
            // std::tie(rotationSpeedXY,rotationSpeedZ,std::ignore) = setting.GetTrackbarValues();image用
            video.SetFramefate(framerate);
        }
        break;
    
    default:
        break;
    }
}

void MouseCallback(int event, int x, int y, int flags, void *userdata)
{
    if (event == cv::EVENT_LBUTTONDOWN) {
        //スライダー操作開始
        if(uiVisibility == true){
            slider.JudgeDrag(x, y);
        }
        if(slider.Dragged() == true){
            video.Stop();
        }
        //回転操作開始
        else
        {
            previousMousePos.x = x - scWidth / 2;
            previousMousePos.y = y - scHeight / 2;
            shouldMouseRotation = true;
        }
        return;
    }

    if( event == cv::EVENT_LBUTTONUP){
        //スライダーの位置に合わせて動画読み込み
        if(slider.Dragged() == true){
            slider.MouseDrag(x);
            int frame = slider.GetCount();
            
            video.SeekFrame(frame,image.src);
            image.MakeDstimg(axis.x, axis.y, axis.z);
            slider.Draw(image.dst);
            cv::imshow("Main",image.dst);
        }
        slider.ReleaseDrag();
        shouldMouseRotation = false;
        return;
    }

    if(event == cv::EVENT_MOUSEMOVE){
        //マウスの位置に合わせてスライダーUIの表示を変える（動画は読み込まない）
        if( slider.Dragged() == true){
            slider.MouseDrag(x);
            image.MakeDstimg(axis.x, axis.y, axis.z);
            slider.Draw(image.dst);
            cv::imshow("Main",image.dst);
        }
        //画面中央から離れるほど画面の奥方向に対する回転を強くする
        //x軸から離れるほど左右方向の視線変更を小さくする
        //y軸から離れるほど上下方向の視線変更を小さくする
        else if(shouldMouseRotation){
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
            if(uiVisibility) slider.Draw(image.dst);
            cv::imshow("Main",image.dst);

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
            image.Zoom(5.0f);
        }

        image.MakeDstimg(axis.x, axis.y, axis.z);
        if(uiVisibility) slider.Draw(image.dst);
        cv::imshow("Main",image.dst);
    }
}

int main(int argc, char **argv) {
    if(argc == 1){
        std::cout << "please input filename";
        exit(1);
    }
    if(argc == 3){
        ReadSettingfile(argv[2]);
    }

    video.Init(argv[1],image.src);
    image.SetDstMat(cv::Size(scWidth, scHeight));

    InitSlider();

    setting.SetInitialValues(rotationSpeedXY, rotationSpeedZ, video.Framerate());

    rotationSpeedXY = 1.0f / (scWidth * scHeight);
    if(scWidth > scHeight){
        rotationSpeedZ = 1.0f / ((scWidth / 2) * (scWidth / 2));
    }
    else{
        rotationSpeedZ = 1.0f / ((scHeight / 2) * (scHeight / 2));
    }

    image.MakeDstimg(axis.x, axis.y, axis.z);
    if(uiVisibility) slider.Draw(image.dst, video.CurrentFrame());
    cv::namedWindow("Main");
    cv::imshow("Main",image.dst);
    cv::setMouseCallback("Main",MouseCallback);

    std::cout << "image  size: " << image.src.size << std::endl;
    std::cout << "window size: " << image.dst.size << std::endl;

    while(true){
        int keyI;
        if( video.Playing() == true){
            keyI = video.Process(image.src);
            // keyI = ProcessVideo();
        }
        else{
            keyI = cv::waitKey(0);
        }
        char keyC = (char)keyI;
        //escape(27)が押されたとき
        if(keyI == 27){
            break;
        }

        OperateByKey(keyC);
        OperateVideoByKeyInput(keyC);

        image.MakeDstimg(axis.x, axis.y, axis.z);
        if(uiVisibility) slider.Draw(image.dst, video.CurrentFrame());
        cv::imshow("Main",image.dst);
    }
 
    return 0;
}