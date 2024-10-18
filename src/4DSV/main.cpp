#include <opencv2/opencv.hpp>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>

#include "../common/slider.hpp"

namespace fs = std::filesystem;

typedef cv::Point3_<uint8_t> Pixel;

const double pi = acos(-1);
const double dpi = pi * 2;

//視野角
float fovy = 70.0f;
const float maxfovy = 90.0f;
const float minfovy = 25.0f;

//デフォルトスクリーンサイズ
int scHeight = 600;
int scWidth = 600;

//回転軸
cv::Vec3d xAxis(1.0f,0.0f,0.0f);
cv::Vec3d yAxis(0.0f,1.0f,0.0f);
cv::Vec3d zAxis(0.0f,0.0f,1.0f);
//回転行列
cv::Matx33d xRotateMat(0.0f);
cv::Matx33d yRotateMat(0.0f);
cv::Matx33d zRotateMat(0.0f);
cv::Matx33d zyxRotateMat(0.0f);

//マウスドラッグによる回転
cv::Point2i previousMousePos, currentMousePos , diffMousePos;
bool shouldMouseRotation = false;
int rotationCorrection = 100000;//後で（スクリーンサイズ / 2）**2を入れる

//動画
bool playing = false;
bool reachEnd = false;
int framecount;
int currentframe;
float frameInterval;
float deltaTime;
cv::TickMeter tick;

//4dsv
cv::Point3i dimension;
cv::Point3i position;
std::vector<std::string> filepathes;

cv::VideoCapture video;
cv::Mat srcimg;
cv::Mat dstimg;

cv::Scalar fontcolor(255,255,255);

//スライダー
const int sliderWidth = 150;
const int sliderHeight = 10;
const int sliderPaddingWidth = 10;//スライダーを画面端からどれだけ離すか
const int sliderPaddingHeight = 20;
const int sliderCollisionPadding = 10;//スライダーの操作判定を見た目より少し大きくする
const int sliderStartWidth = scWidth - sliderWidth - sliderPaddingWidth;
const int sliderStartHeight = scHeight - sliderHeight - sliderPaddingHeight;

Slider slider;

bool uiVisibility = true;


//元の全方位画像から普通の透視投影の画像を作る
void MakeDstimg(cv::Mat& dstimg, const cv::Mat& srcimg)
{
    const int srcWidth = srcimg.cols;
    const int srcHeight = srcimg.rows;
    const int channels = srcimg.channels();

    const int dstWidth = dstimg.cols;
    const int dstHeight = dstimg.rows;

    cv::Vec3f lefttop, righttop, leftbottom, rightbottom;
    float aspect = (float)dstWidth / (float)dstHeight;
    float correctionX = std::sin((aspect * fovy / 2.0 * pi) / 180.0);
    float correctionY = std::sin((fovy / 2.0 * pi) / 180.0);

    lefttop = -xAxis * correctionX + yAxis * correctionY + zAxis;
    righttop = xAxis * correctionX + yAxis * correctionY + zAxis;
    leftbottom = -xAxis * correctionX - yAxis * correctionY + zAxis;
    rightbottom = xAxis * correctionX - yAxis * correctionY + zAxis;

    dstimg.forEach<Pixel>
    ([&srcimg, &lefttop, &leftbottom, &righttop, &rightbottom,
     srcWidth, srcHeight, channels, dstWidth, dstHeight]
    (Pixel& pixel, const int position[]) -> void
    {
        cv::Vec3f coord = lefttop + position[1] * (righttop - lefttop) / (dstWidth - 1)
                                  + position[0] * (leftbottom - lefttop) / (dstHeight - 1);

        coord = cv::normalize(coord);
        float theta = std::acos(coord.val[1]);
        float phi = coord.val[0] != 0 ? std::atan2(coord.val[0],coord.val[2]) : 0.0f;
            
        int srcWIdx = (int)((pi + phi) / dpi * (srcWidth - 1));
        int srcHIdx = (int)( theta / pi * (srcHeight - 1));

        int srcIdx = srcHIdx * srcWidth * channels + srcWIdx * channels;
        pixel.x = srcimg.data[srcIdx];
        pixel.y = srcimg.data[srcIdx + 1];
        pixel.z = srcimg.data[srcIdx + 2];
    });
}

void Rotate(float roll, float pitch, float yaw)
{
    cv::Rodrigues(xAxis * roll, xRotateMat);
    cv::Rodrigues(yAxis * pitch,yRotateMat);
    cv::Rodrigues(zAxis * yaw,  zRotateMat);

    zyxRotateMat = xRotateMat * yRotateMat * zRotateMat;
    cv::Matx31d zAxisMat(zAxis);
    cv::Matx31d xAxisMat(xAxis);
    zAxisMat = zyxRotateMat * zAxisMat;
    xAxisMat = zyxRotateMat * xAxisMat;

    zAxis = cv::Vec3d(zAxisMat.val);
    xAxis = cv::Vec3d(xAxisMat.val);

    zAxis = cv::normalize(zAxis);
    xAxis = cv::normalize(xAxis);
    yAxis = zAxis.cross(xAxis);
    yAxis = cv::normalize(yAxis);
}

void StepForward()
{
    if(reachEnd){
        playing = false;
        return;
    }
    video.read(srcimg);
    currentframe++;
    if(currentframe >= framecount - 1){
        reachEnd = true;
    }
    return;
}

void SeekFrame(int frame)
{
    playing = false;
    reachEnd = false;
    currentframe = frame;
    if(currentframe < 0){
        currentframe = 0;
    }
    else if(currentframe >= framecount - 1){
        currentframe = framecount - 1;
        reachEnd = true;
    }
    video.set(cv::CAP_PROP_POS_FRAMES, currentframe);
    video.read(srcimg);
}

void InitVideo(std::string filename, cv::VideoCapture* video, cv::Mat* img)
{
    video->open(filename);
    framecount = video->get(cv::CAP_PROP_FRAME_COUNT);
    float framerate = video->get(cv::CAP_PROP_FPS);
    frameInterval = 1000.0f / framerate;
    currentframe = 0;
    video->read(*img);
}

int ProccessVideo()
{
    tick.stop();
    deltaTime += tick.getTimeMilli();
    tick.reset();
    tick.start();

    int key = cv::pollKey();
    
    if(deltaTime >= frameInterval){
        deltaTime -= frameInterval;
        StepForward();
    }
    return key;
}

void ChangeVideoPos(int x, int y, int z)
{
    int movedX = position.x + x;
    int movedY = position.y + y;
    int movedZ = position.z + z;

    if(movedX < 0 || movedX >= dimension.x ||
       movedY < 0 || movedY >= dimension.y ||
       movedZ < 0 || movedZ >= dimension.z)
    {return;}

    position.x = movedX;
    position.y = movedY;
    position.z = movedZ;
    int fileIdx = position.z * dimension.y * dimension.x + position.y * dimension.x + position.x;
    video.open(filepathes[fileIdx]);
    video.set(cv::CAP_PROP_POS_FRAMES, currentframe);
    video.read(srcimg);
}

void ReadVideoInformationFile(char * filename)
{
    std::ifstream config(filename);
    std::string directorypath;
    std::string extension;

    config >> directorypath;
    config >> extension;
    config >> dimension.x >> dimension.y >> dimension.z;
    config >> position.x >> position.y >> position.z;

    if(fs::exists(directorypath) == false){
        printf("video directory not Found\n");
        exit(1);
    }
    
    //extensionが一致するファイルのパスを取得する
    fs::directory_iterator iter(directorypath), end;
    std::error_code errCode;
    for(;iter != end; iter.increment(errCode)){
        fs::directory_entry entry = *iter;
        std::string filetype = entry.path().extension().string();
        if(filetype == ("."+ extension)){
            filepathes.push_back(entry.path().string());
        }
    }

    if(dimension.x * dimension.y * dimension.z != (int)filepathes.size()){
        std::cout << "dimension is strange" << std::endl;
        exit(1);
    }

    std::sort(filepathes.begin(),filepathes.end());

    int fileIdx = position.z * dimension.y * dimension.x + position.y * dimension.x + position.x;
    InitVideo(filepathes[fileIdx],&video,&srcimg);

    dstimg = cv::Mat(cv::Size(scWidth, scHeight), srcimg.type(), cv::Scalar::all(0));
}

void OperateByKey(char key)
{
    switch (key)
    {
    case 'a'://左へ
        Rotate(0,-0.05f,0);break;
    case 'd'://右へ
        Rotate(0,0.05f,0);break;
    case 'w'://上へ
        Rotate(-0.05f,0,0);break;
    case 's'://下へ
        Rotate(0.05f,0,0);break;
    case 'q'://左回転
        Rotate(0,0,-0.05f);break;
    case 'e'://右回転
        Rotate(0,0,0.05f);break;
    
    default:
        break;
    }
}

void OperateVideoByKeyInput(char key)
{
    switch (key)
    {
    case ' '://space
        playing = !playing;
        if(playing == true){
            tick.start();
        }
        else{
            tick.stop();
            tick.reset();
        }
        deltaTime = 0.0f;
        break;

    case 13://Enter
        playing = false;
        StepForward();
        break;

    case 8:  //backspace
    case 127://delete
        SeekFrame(currentframe - 1);
        break;

    case '0':
        SeekFrame(0);
        break;

    case 'v':
        uiVisibility = !uiVisibility;
        break;

    //ズームイン、ズームアウト
    case '+':
        fovy -= 5.0f;
        if(fovy < minfovy){
            fovy = minfovy;
        }
        break;

    case '-':
        fovy += 5.0f;
        if(fovy > maxfovy){
            fovy = maxfovy;
        }
        break;
    
    default:
        break;
    }
}

void OperateVideoSwitch(char key)
{
    int x = 0;
    int y = 0;
    int z = 0;
    switch (key)
    {
    case 'I':
        x = -1; break;
    case 'i':
        x = 1;  break;
    case 'J':
        y = -1; break;
    case 'j':
        y = 1;  break;
    case 'K':
        z = -1; break;
    case 'k':
        z = 1;  break;

    default:
        return;
    }
    ChangeVideoPos(x, y, z);
}

//画面左上に情報を表示
void DrawTextInfo()
{
    if(uiVisibility == false) return;
    std::stringstream ssDim,ssPos;
    ssDim << "dim" << dimension;
    cv::putText(dstimg,ssDim.str(),cv::Point(10,20),cv::FONT_HERSHEY_PLAIN,1.5,fontcolor,2.0);
    ssPos << "pos" << position;
    cv::putText(dstimg,ssPos.str(),cv::Point(10,40),cv::FONT_HERSHEY_PLAIN,1.5,fontcolor,2.0);
}

void MouseCallback(int event, int x, int y, int flags, void *userdata)
{
    if (event == cv::EVENT_LBUTTONDOWN) {
        //スライダー操作開始
        if(uiVisibility == true){
            slider.JudgeDrag(x, y);
        }
        if(slider.Dragged() == true){
            playing = false;
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
            reachEnd = false;
            slider.MouseDrag(x);
            int frame = slider.GetCount();
            
            SeekFrame(frame);
            MakeDstimg(dstimg,srcimg);
            DrawTextInfo();
            slider.Draw(dstimg,frame);
            cv::imshow("Main",dstimg);
        }
        slider.ReleaseDrag();
        shouldMouseRotation = false;
        return;
    }

    if(event == cv::EVENT_MOUSEMOVE){
        //マウスの位置に合わせてスライダーUIの表示を変える（動画は読み込まない）
        if( slider.Dragged() == true){
            slider.MouseDrag(x);
            currentframe = slider.GetCount();
            MakeDstimg(dstimg,srcimg);
            DrawTextInfo();
            slider.Draw(dstimg,currentframe);
            cv::imshow("Main",dstimg);
        }
        //画面中央から離れるほど画面の奥方向に対する回転を強くする
        //x軸から離れるほど左右方向の視線変更を小さくする
        //y軸から離れるほど上下方向の視線変更を小さくする
        else if(shouldMouseRotation){
            currentMousePos.x = x - scWidth / 2;
            currentMousePos.y = y - scHeight / 2;
            diffMousePos = currentMousePos - previousMousePos;

            float roll = -diffMousePos.y * (scWidth - std::abs(currentMousePos.x) * 2) / (float)(scWidth * scWidth);
            float pitch = -diffMousePos.x * (scHeight - std::abs(currentMousePos.y) * 2) / (float)(scHeight * scHeight);
            float yaw = previousMousePos.cross(currentMousePos) / rotationCorrection;

            Rotate(roll,pitch,yaw);
            MakeDstimg(dstimg,srcimg);
            DrawTextInfo();
            slider.Draw(dstimg,currentframe);
            cv::imshow("Main",dstimg);

            previousMousePos.x = currentMousePos.x;
            previousMousePos.y = currentMousePos.y;
        }
        return;
    }

    //マウスのホイールを動かすとズームイン、ズームアウトする
    //Windows版のみ？
    if(event == cv::EVENT_MOUSEWHEEL){
        if(flags > 0){
            fovy += 5.0f;
        }
        else{
            fovy -= 5.0f;
        }
        fovy = std::clamp(fovy, minfovy, maxfovy);

        MakeDstimg(dstimg,srcimg);
        DrawTextInfo();
        slider.Draw(dstimg,currentframe);
        cv::imshow("Main",dstimg);
    }
}

int main(int argc, char **argv) {
    std::cout << cv::getVersionString() << std::endl;
    if(argc == 1){
        std::cout << "please input filename";
        exit(1);
    }
    ReadVideoInformationFile(argv[1]);
    slider.SetShape(cv::Point2i(sliderStartWidth,sliderStartHeight),cv::Size2i(sliderWidth,sliderHeight));
    slider.SetTotalCount(framecount);

    if(scWidth > scHeight){
        rotationCorrection = (scWidth / 2) * (scWidth / 2);
    }
    else{
        rotationCorrection = (scHeight / 2) * (scHeight / 2);
    }

    std::cout << srcimg.size << std::endl;
    MakeDstimg(dstimg,srcimg);
    DrawTextInfo();
    slider.Draw(dstimg,0);
    cv::namedWindow("Main");
    cv::imshow("Main",dstimg);
    cv::setMouseCallback("Main",MouseCallback);
    while(true){
        int keyI;
        if( playing == true){
            keyI = ProccessVideo();
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
        OperateVideoSwitch(keyC);

        MakeDstimg(dstimg,srcimg);
        DrawTextInfo();
        if(uiVisibility) slider.Draw(dstimg,currentframe);
        cv::imshow("Main",dstimg);
    }
 
    return 0;
}