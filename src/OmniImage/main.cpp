#include <opencv2/opencv.hpp>
#include <cmath>
#include "../common/axis.hpp"

typedef cv::Point3_<uint8_t> Pixel;

const double pi = acos(-1);
const double dpi = pi * 2;
const double deg2rad = pi / 180.0;
const double rad2deg = 180.0 / pi;

//視野角
float fovy = 60.0f;
const float maxfovy = 90.0f;
const float minfovy = 30.0f;
//スクリーンサイズ
int scHeight = 500;
int scWidth = 500;
//回転軸
cv::Vec3d xAxis(1.0,0.0,0.0);
cv::Vec3d yAxis(0.0,1.0,0.0);
cv::Vec3d zAxis(0.0,0.0,1.0);

const float rotationAngle = 0.05f;


//回転行列
cv::Matx33d xRotateMat(0.0);
cv::Matx33d yRotateMat(0.0);
cv::Matx33d zRotateMat(0.0);
cv::Matx33d zyxRotateMat(0.0);

//マウス回転用変数
cv::Point2i previousMousePos, currentMousePos , diffMousePos;
bool shouldMouseRotation = false;
float rotationCorrection = 0;

cv::Mat srcimg, dstimg;

void MakeDstimg()
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
    ([&lefttop, &leftbottom, &righttop, &rightbottom,
     srcWidth, srcHeight, channels, dstWidth, dstHeight]
    (Pixel& pixel, const int position[]) -> void
    {
        cv::Vec3f coord = lefttop + position[1] * (righttop - lefttop) / (dstWidth - 1)
                                  + position[0] * (leftbottom - lefttop) / (dstHeight - 1);

        coord = cv::normalize(coord);
        float theta = std::acos(coord.val[1]);
        float phi = coord.val[0] != 0 ? std::atan2(coord.val[0],coord.val[2]) : 0.0f;
            
        int srcWIdx = round((pi + phi) / dpi * (srcWidth - 1));
        int srcHIdx = round( theta / pi * (srcHeight - 1));

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

void Zoom(float degree)
{
    fovy += degree;
    fovy = std::clamp(fovy, minfovy, maxfovy);
}

void Reset()
{
    xAxis = cv::Vec3d(1.0,0.0,0.0);
    yAxis = cv::Vec3d(0.0,1.0,0.0);
    zAxis = cv::Vec3d(0.0,0.0,1.0);
}

void OperateByKey(char key)
{
    switch (key)
    {
    //回転操作
    case 'a'://左へ
        Rotate(0,-rotationAngle,0);break;
    case 'd'://右へ
        Rotate(0,rotationAngle,0);break;
    case 'w'://上へ
        Rotate(-rotationAngle,0,0);break;
    case 's'://下へ
        Rotate(rotationAngle,0,0);break;
    case 'q'://左回転
        Rotate(0,0,-rotationAngle);break;
    case 'e'://右回転
        Rotate(0,0,rotationAngle);break;

    //ズームイン・ズームアウト
    case '+':
        Zoom(-5.0f);break;
    case '-':
        Zoom(+5.0f);break;

    //視線方向をリセット
    case 'r':
        Reset();break;
    
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

            float roll = -diffMousePos.y * (scWidth - std::abs(currentMousePos.x) * 2) / (float)(scWidth * scWidth);
            float pitch = -diffMousePos.x * (scHeight - std::abs(currentMousePos.y) * 2) / (float)(scHeight * scHeight);
            float yaw = previousMousePos.cross(currentMousePos) / rotationCorrection;

            Rotate(roll,pitch,yaw);
            MakeDstimg();
            cv::imshow("dst",dstimg);

            previousMousePos.x = currentMousePos.x;
            previousMousePos.y = currentMousePos.y;
        }
        return;
    }

    //マウスのホイールを動かすとズームイン、ズームアウトする
    //Windows版のみ？
    if(event == cv::EVENT_MOUSEWHEEL){
        if(flags > 0){
            Zoom(5.0f);
        }
        else{
            Zoom(-5.0f);
        }
        MakeDstimg();
        cv::imshow("dst",dstimg);
    }
}

int main(int argc, char **argv) {
    if(argc == 1){
        std::cout << "please input filename";
        exit(1);
    }
    srcimg = cv::imread(argv[1]);

    if(argc == 4){
        scWidth = std::stoi(argv[2]);
        scHeight = std::stoi(argv[3]);
    }

    if(scWidth > scHeight){
        rotationCorrection = (scWidth / 2) * (scWidth / 2);
    }
    else{
        rotationCorrection = (scHeight / 2) * (scHeight / 2);
    }

    dstimg = cv::Mat(cv::Size(scWidth, scHeight), srcimg.type(), cv::Scalar::all(0));
    std::cout << srcimg.size << std::endl;
    MakeDstimg();
    // DrawAxis(dstimg, xAxis, yAxis, zAxis, cv::Point(250, 250));
    cv::namedWindow("dst");
    cv::setMouseCallback("dst",MouseCallback);
    cv::imshow("dst",dstimg);

    while(true){
        int keyI = cv::waitKey(0);
        char keyC = (char)keyI;

        //ウィンドウが閉じられた時(-1),またはescape(27)が押されたとき
        if(keyI == -1 || keyI == 27){
            break;
        }

        OperateByKey(keyC);

        MakeDstimg();
        // DrawAxis(dstimg, xAxis, yAxis, zAxis, cv::Point(250, 250));
        cv::imshow("dst",dstimg);
    }
    return 0;
}