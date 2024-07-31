#include <opencv2/opencv.hpp>
#include <cmath>
#include <chrono>

typedef cv::Point3_<uint8_t> Pixel;
typedef std::chrono::_V2::system_clock::time_point Time;

// constexpr const double pi = 3.141592653589793;
const double pi = acos(-1);
const double dpi = pi * 2;


//視野角
const float fov = 80.0f;
//デフォルトスクリーンサイズ
int scHeight = 500;
int scWidth = 500;

//回転軸
cv::Vec3d xAxis(1.0f,0.0f,0.0f);
cv::Vec3d yAxis(0.0f,1.0f,0.0f);
cv::Vec3d zAxis(0.0f,0.0f,1.0f);
//回転行列
cv::Matx33d xRotateMat(0.0f);
cv::Matx33d yRotateMat(0.0f);
cv::Matx33d zRotateMat(0.0f);
cv::Matx33d zyxRotateMat(0.0f);

float sensitivityXY = 0.1f;
float sensitivityZ = 0.1f;

//動画
bool playing = false;
bool reachEnd = false;
int framecount;
int currentframe;
float framerate;
float frameInterval;
float deltaTime;
int waitTime;
std::chrono::duration<float, std::milli> elapsed;
Time previousTime;
Time currentTime;

cv::VideoCapture mainVideo;
cv::Mat srcimg;

void InitRayVector(cv::Mat& rayvector)
{
    cv::Point3d lefttop, righttop, leftbottom, rightbottom;
    float aspect;
    
    aspect = (float)scWidth / (float)scHeight;

    lefttop.x = std::sin((aspect * fov / 2.0 * pi) / 180.0) * -1;
    lefttop.y = std::sin((fov / 2.0 * pi) / 180.0);

    // lefttop =  xAxis * std::sin(aspect * fov / 2.0 * deg2rad);
    
    
    righttop.x = lefttop.x * -1;
    righttop.y = lefttop.y;
    
    leftbottom.x = lefttop.x;
    leftbottom.y = lefttop.y * -1;

    rightbottom.x = righttop.x;
    rightbottom.y = righttop.y * -1;
    
    for(int hIdx = 0; hIdx < scHeight; hIdx++){
        float y = lefttop.y + hIdx * (leftbottom.y - lefttop.y) / (scHeight - 1);
        for(int wIdx = 0; wIdx < scWidth; wIdx++){
            float x = lefttop.x + wIdx * (righttop.x - lefttop.x) / (scWidth - 1);
            float z = 1.0f;
            float length = std::sqrt(x*x + y*y + z*z);
            rayvector.at<cv::Vec3f>(hIdx,wIdx)[0] = x / length;
            rayvector.at<cv::Vec3f>(hIdx,wIdx)[1] = y / length;
            rayvector.at<cv::Vec3f>(hIdx,wIdx)[2] = z / length;
        }
    }
}

void MakeDstimg(cv::Mat& dstimg, const cv::Mat& rayvector, const cv::Mat& srcimg)
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
    ([&rayvector, &srcimg, &lefttop, &leftbottom, &righttop, &rightbottom,
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
        // pixel = srcimg.at<Pixel>(srcHIdx,srcWIdx);
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

void MouseCallback(int event, int x, int y, int flags, void *userdata)
{
    if (event == cv::EVENT_LBUTTONDOWN) {
        std::cout << "Left mouse button is pressed." << std::endl;
    }
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

void StepForward(cv::VideoCapture* video, cv::Mat* img)
{
    if(reachEnd){
        playing = false;
        return;
    }
    video->read(*img);
    currentframe++;
    if(currentframe >= framecount - 1){
        reachEnd = true;
    }
    return;
}

void StepBackWard(cv::VideoCapture* video, cv::Mat* img)
{
    playing = false;
    reachEnd = false;
    currentframe--;
    if(currentframe < 0){
        currentframe = 0;
    }
    video->set(cv::CAP_PROP_POS_FRAMES, currentframe);
    video->read(*img);
}

void OperateVideoByKeyInput(char key, cv::VideoCapture* video, cv::Mat* img)
{
    switch (key)
    {
    case ' '://space
        playing = !playing;
        previousTime = std::chrono::high_resolution_clock::now();
        deltaTime = 0.0f;
        break;

    case 13://Enter
        playing = false;
        StepForward(video, img);
        break;

    case 8://backspace
        StepBackWard(video, img);
    
    default:
        break;
    }
}

void InitVideo(char* filename, cv::VideoCapture* video, cv::Mat* img)
{
    video->open(filename);
    framecount = video->get(cv::CAP_PROP_FRAME_COUNT);
    framerate = video->get(cv::CAP_PROP_FPS);
    frameInterval = 1000.0f / framerate;
    currentframe = 0;
    video->read(*img);
}

int ProccessVideo(cv::VideoCapture* video, cv::Mat* img)
{
    currentTime = std::chrono::high_resolution_clock::now();
    elapsed = currentTime - previousTime;
    deltaTime += elapsed.count();
    previousTime = currentTime;

    int key = cv::pollKey();
    if(deltaTime >= frameInterval){
        deltaTime -= frameInterval;
        // video->read(*img);
        StepForward(video,img);
    }
    return key;
}
// int ProccessVideo(cv::VideoCapture* video, cv::Mat* img)
// {
//     currentTime = std::chrono::high_resolution_clock::now();
//     elapsed = currentTime - previousTime;
//     deltaTime += elapsed.count();
//     previousTime = currentTime;

//     int key = cv::waitKey(1);
//     if(deltaTime >= frameInterval){
//         deltaTime -= frameInterval;
//         // video->read(*img);
//         StepForward(video,img);
//     }
//     return key;
// }


int main(int argc, char **argv) {
    if(argc == 1){
        std::cout << "please input filename";
        exit(1);
    }
    if(argc == 4){
        scWidth = atoi(argv[2]);
        scHeight = atoi(argv[3]);
    }
    // cv::Mat srcimg;
    // cv::VideoCapture video(argv[1]);
    InitVideo(argv[1],&mainVideo,&srcimg);

    cv::Mat rayvector = cv::Mat(cv::Size(scWidth,scHeight),CV_32FC3,cv::Scalar::all(0.0f));
    cv::Mat dstimg(cv::Size(scWidth, scHeight), srcimg.type(), cv::Scalar::all(0));
    std::cout << srcimg.size << std::endl;
    MakeDstimg(dstimg,rayvector,srcimg);

    cv::namedWindow("Main");
    cv::setMouseCallback("Main",MouseCallback);

    // std::cout << "経過時間: " << elapsed.count() << " ms" << std::endl;
    cv::imshow("Main",dstimg);
    previousTime =  std::chrono::high_resolution_clock::now();
    deltaTime = 0.0f;
    while(true){
        currentTime = std::chrono::high_resolution_clock::now();
        int keyI;
        if( playing == true){
            keyI = ProccessVideo(&mainVideo,&srcimg);
        }
        else{
            keyI = cv::waitKey(0);
        }
        char keyC = (char)keyI;
        // std::cout << keyI << std::endl;
        //ウィンドウが閉じられた時(-1),またはescape(27)が押されたとき
        if(keyI == 27){
            break;
        }

        RotateByKeyInput(keyC);
        OperateVideoByKeyInput(keyC, &mainVideo, &srcimg);

        MakeDstimg(dstimg,rayvector,srcimg);
        cv::imshow("Main",dstimg);
        
    }
 
    return 0;
}