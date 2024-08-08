#include <opencv2/opencv.hpp>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>

namespace fs = std::filesystem;

typedef cv::Point3_<uint8_t> Pixel;

const double pi = acos(-1);
const double dpi = pi * 2;

//視野角
const float fov = 80.0f;
//デフォルトスクリーンサイズ
int scHeight = 600;
int scWidth = 600;
int scHeightSub = 300;
int scWidthSub = 300; 

//回転軸
cv::Vec3d xAxis(1.0f,0.0f,0.0f);
cv::Vec3d yAxis(0.0f,1.0f,0.0f);
cv::Vec3d zAxis(0.0f,0.0f,1.0f);
//回転行列
cv::Matx33d xRotateMat(0.0f);
cv::Matx33d yRotateMat(0.0f);
cv::Matx33d zRotateMat(0.0f);
cv::Matx33d zyxRotateMat(0.0f);


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
std::vector<std::string> SubFilepathes;

cv::VideoCapture mainVideo, subVideo;
cv::Mat srcimg, subSrcimg;
cv::Mat dstimg, subDstimg;

cv::Scalar fontcolor(255,255,255);

//スライダー
const int sliderWidth = 150;
const int sliderHeight = 10;
const int sliderPaddingWidth = 10;//スライダーを画面端からどれだけ離すか
const int sliderPaddingHeight = 20;
const int sliderCollisionPadding = 10;//スライダーの操作判定を見た目より少し大きくする
const int sliderStartWidth = scWidth - sliderWidth - sliderPaddingWidth;
const int sliderStartHeight = scHeight - sliderHeight - sliderPaddingHeight;

cv::Rect sliderBaseArea(sliderStartWidth, sliderStartHeight, sliderWidth, sliderHeight);
cv::Rect sliderMoveArea(sliderStartWidth, sliderStartHeight, 0, sliderHeight);
cv::Scalar sliderColor(50,255,50);
bool sliderDragged = false;

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
    float correctionX = std::sin((aspect * fov / 2.0 * pi) / 180.0);
    float correctionY = std::sin((fov / 2.0 * pi) / 180.0);

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
    mainVideo.read(srcimg);
    subVideo.read(subSrcimg);
    currentframe++;
    if(currentframe >= framecount - 1){
        reachEnd = true;
    }
    return;
}

void StepBackWard()
{
    playing = false;
    reachEnd = false;
    currentframe--;
    if(currentframe < 0){
        currentframe = 0;
    }
    mainVideo.set(cv::CAP_PROP_POS_FRAMES, currentframe);
    mainVideo.read(srcimg);
    subVideo.set(cv::CAP_PROP_POS_FRAMES, currentframe);
    subVideo.read(subSrcimg);
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
    mainVideo.open(filepathes[fileIdx]);
    mainVideo.set(cv::CAP_PROP_POS_FRAMES, currentframe);
    mainVideo.read(srcimg);
    subVideo.open(SubFilepathes[fileIdx]);
    subVideo.set(cv::CAP_PROP_POS_FRAMES, currentframe);
    subVideo.read(subSrcimg);
}

void ReadVideoInformationFile(char * filename)
{
    std::ifstream config(filename);
    std::string maindir;
    std::string subdir;
    std::string extension;
    int framerate;

    config >> maindir;
    config >> extension;
    config >> dimension.x >> dimension.y >> dimension.z;
    config >> position.x >> position.y >> position.z;
    config >> framerate;
    config >> subdir;

    if(fs::exists(maindir) == false || fs::exists(subdir) == false){
        printf("video directory not Found\n");
        exit(1);
    }
    
    //extensionが一致するファイルのパスを取得する
    {
        fs::directory_iterator iter(maindir), end;
        std::error_code errCode;
        for(;iter != end; iter.increment(errCode)){
            fs::directory_entry entry = *iter;
            std::string filetype = entry.path().extension().string();
            if(filetype == ("."+ extension)){
                filepathes.push_back(entry.path().string());
            }
        }
    }
    {
        fs::directory_iterator iter(subdir), end;
        std::error_code errCode;
        for(;iter != end; iter.increment(errCode)){
            fs::directory_entry entry = *iter;
            std::string filetype = entry.path().extension().string();
            if(filetype == ("."+ extension)){
                SubFilepathes.push_back(entry.path().string());
            }
        }
    }

    if(dimension.x * dimension.y * dimension.z != (int)filepathes.size()){
        std::cout << "dimension is strange" << std::endl;
        exit(1);
    }

    std::sort(filepathes.begin(),filepathes.end());
    std::sort(SubFilepathes.begin(),SubFilepathes.end());

    int fileIdx = position.z * dimension.y * dimension.x + position.y * dimension.x + position.x;
    InitVideo(filepathes[fileIdx],&mainVideo,&srcimg);
    InitVideo(SubFilepathes[fileIdx], &subVideo, &subSrcimg);

    dstimg = cv::Mat(cv::Size(scWidth, scHeight), srcimg.type(), cv::Scalar::all(0));
    subDstimg = cv::Mat(cv::Size(scWidthSub, scHeightSub), srcimg.type(), cv::Scalar::all(0));
}

void RotateByKeyInput(char key)
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
        // previousTime = std::chrono::high_resolution_clock::now();
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

    case 8://backspace
    case 127:
        StepBackWard();
        break;

    case '0':
        mainVideo.set(cv::CAP_PROP_POS_FRAMES,0);
        subVideo.set(cv::CAP_PROP_POS_FRAMES,0);
        currentframe = 0;
        reachEnd = false;
        mainVideo.read(srcimg);
        subVideo.read(subSrcimg);
        break;

    case 'v':
        uiVisibility = !uiVisibility;
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
    case 'h':
        x = -1; break;
    case 'l':
        x = 1;  break;
    case 'j':
        y = -1; break;
    case 'k':
        y = 1;  break;
    case ',':
        z = -1; break;
    case '.':
        z = 1;  break;

    default:
        return;
    }
    ChangeVideoPos(x, y, z);
    std::cout << "position" << position << std::endl;
}

void DrawTextInfo()
{
    if(uiVisibility == false) return;
    std::stringstream ssDim,ssPos;
    ssDim << "dim" << dimension;
    cv::putText(dstimg,ssDim.str(),cv::Point(10,20),cv::FONT_HERSHEY_PLAIN,1.5,fontcolor,2.0);
    ssPos << "pos" << position;
    cv::putText(dstimg,ssPos.str(),cv::Point(10,40),cv::FONT_HERSHEY_PLAIN,1.5,fontcolor,2.0);
}

void DrawSlider()
{
    if(uiVisibility == false) return;
    float progressRate = (float)currentframe / (framecount - 1);
    float barlength = sliderWidth * progressRate;
    sliderMoveArea.width = barlength;
    cv::rectangle(dstimg,sliderBaseArea,fontcolor,-1);
    cv::rectangle(dstimg,sliderMoveArea,sliderColor,-1);
    cv::putText(dstimg,std::to_string(currentframe),cv::Point(sliderStartWidth, sliderStartHeight - 10),
                cv::FONT_HERSHEY_PLAIN,1.5,fontcolor,2.0);
}

void MouseCallback(int event, int x, int y, int flags, void *userdata)
{
    if (event == cv::EVENT_LBUTTONDOWN) {
        if(x >= sliderStartWidth - sliderCollisionPadding && x <= sliderStartWidth + sliderWidth + sliderCollisionPadding &&
           y >= sliderStartHeight - sliderCollisionPadding && y <= sliderStartHeight + sliderHeight + sliderCollisionPadding &&
           uiVisibility == true){
                playing = false;
                sliderDragged = true;
                return;
        }
    }
    else if( event == cv::EVENT_LBUTTONUP){
        if(sliderDragged == true){
            x -= sliderStartWidth;
            reachEnd = false;
            if(x <= 0){
                currentframe = 0;
            }
            else if( x >= sliderWidth){
                currentframe = framecount - 1;
                reachEnd = true;
            }
            else{
                currentframe = (framecount - 1) * x / sliderWidth;
            }
            mainVideo.set(cv::CAP_PROP_POS_FRAMES,currentframe);
            subVideo.set(cv::CAP_PROP_POS_FRAMES,currentframe);
            mainVideo.read(srcimg);
            subVideo.read(subSrcimg);
            MakeDstimg(dstimg,srcimg);
            MakeDstimg(subDstimg,subSrcimg);
            DrawTextInfo();
            DrawSlider();
            cv::imshow("Main",dstimg);
            cv::imshow("Sub",subDstimg);
            sliderDragged = false;
        }
    }
    else if( sliderDragged == true){
        x -= sliderStartWidth;
        if(x <= 0){
            currentframe = 0;
        }
        else if( x >= sliderWidth){
            currentframe = framecount - 1;
        }
        else{
            currentframe = (framecount - 1) * x / sliderWidth;
        }
        MakeDstimg(dstimg,srcimg);
        MakeDstimg(subDstimg,subSrcimg);
        DrawTextInfo();
        DrawSlider();
        cv::imshow("Main",dstimg);
        cv::imshow("Sub",subDstimg);
    }
}

int main(int argc, char **argv) {
    if(argc == 1){
        std::cout << "please input filename";
        exit(1);
    }
    ReadVideoInformationFile(argv[1]);

    std::cout << srcimg.size << std::endl;
    MakeDstimg(dstimg,srcimg);
    MakeDstimg(subDstimg,subSrcimg);
    DrawTextInfo();
    DrawSlider();
    cv::namedWindow("Main");
    cv::namedWindow("Sub");
    cv::imshow("Main",dstimg);
    cv::imshow("Sub",subDstimg);
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

        RotateByKeyInput(keyC);
        OperateVideoByKeyInput(keyC);
        OperateVideoSwitch(keyC);

        MakeDstimg(dstimg,srcimg);
        MakeDstimg(subDstimg,subSrcimg);
        DrawTextInfo();
        DrawSlider();
        cv::imshow("Main",dstimg);
        cv::imshow("Sub",subDstimg);
        
    }
 
    return 0;
}