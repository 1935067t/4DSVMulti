#include <opencv2/opencv.hpp>
#include <cmath>
#include <chrono>

typedef cv::Point3_<uint8_t> Pixel;
typedef cv::Point3_<float> Vector;

// constexpr const double pi = 3.141592653589793;
const double pi = acos(-1);
const double dpi = pi * 2;
const double deg2rad = pi / 180.0;
const double rad2deg = 180.0 / pi;
//視野角
const float fov = 60.0f;
//スクリーンサイズ
int scHeight = 500;
int scWidth = 500;
//回転軸
cv::Vec3d xAxis(1.0f,0.0f,0.0f);
cv::Vec3d yAxis(0.0f,1.0f,0.0f);
cv::Vec3d zAxis(0.0f,0.0f,1.0f);

float sensitivityXY = 0.1f;
float sensitivityZ = 0.1f;


//回転行列
cv::Matx33d xRotateMat(0.0f);
cv::Matx33d yRotateMat(0.0f);
cv::Matx33d zRotateMat(0.0f);
cv::Matx33d zyxRotateMat(0.0f);

void InitRayVector(cv::Mat& rayvector)
{
    cv::Point3d lefttop, righttop, leftbottom, rightbottom;
    float aspect;
    
    aspect = (float)scWidth / (float)scHeight;

    lefttop.x = std::sin((aspect * fov / 2.0 * pi) / 180.0) * -1;
    lefttop.y = std::sin((fov / 2.0 * pi) / 180.0);

    lefttop =  xAxis * std::sin(aspect * fov / 2.0 * deg2rad);
    
    
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
    // float aspect = (float)scWidth / (float)scHeight;
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
        // srcimg.ptr<Pixel>
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
    // xAxis[2] = zAxis[2] != 0.0f ? (zAxis[0]*xAxis[0] + zAxis[1]*xAxis[1])/zAxis[2] : 1.0f;
    xAxis = cv::normalize(xAxis);
    yAxis = zAxis.cross(xAxis);
    yAxis = cv::normalize(yAxis);
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

int main() {
    cv::Mat img;
    img = cv::imread("../omni_image_er.bmp");

    scHeight = 500;
    scWidth = 500;

    cv::Mat rayvector = cv::Mat(cv::Size(scWidth,scHeight),CV_32FC3,cv::Scalar::all(0.0f));
    cv::Mat dstimg(cv::Size(scWidth, scHeight), img.type(), cv::Scalar::all(0));
    std::cout << img.size << std::endl;
    // InitRayVector(rayvector);
    auto start = std::chrono::high_resolution_clock::now();
    MakeDstimg(dstimg,rayvector,img);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << "経過時間: " << elapsed.count() << " ms" << std::endl;
    cv::imshow("dst",dstimg);
    while(true){
        int keyI = cv::waitKey(0);
        char keyC = (char)keyI;
        std::cout << keyI << std::endl;
        //ウィンドウが閉じられた時(-1),またはescape(27)が押されたとき
        if(keyI == -1 || keyI == 27){
            break;
        }
        if(keyI == 13){
            std::cout << "enter" << std::endl;
        }
        RotateByKeyInput(keyC);

        MakeDstimg(dstimg,rayvector,img);
        cv::imshow("dst",dstimg);
        
    }
    
    
    // Rotate(0.0f,0.1f,0);
    // MakeDstimg(dstimg,rayvector,img);
    // cv::imshow("dst",dstimg);
    // cv::waitKey(0);
    
    // Rotate(0.1f,0.0f,0);
    // MakeDstimg(dstimg,rayvector,img);
    // cv::imshow("dst",dstimg);
    // cv::waitKey(0);
    // Rotate(0.1f,0.0f,0);
    // MakeDstimg(dstimg,rayvector,img);
    // cv::imshow("dst",dstimg);
    // cv::waitKey(0);
    // Rotate(0.1f,0.0f,0);
    // MakeDstimg(dstimg,rayvector,img);
    // cv::imshow("dst",dstimg);
    // cv::waitKey(0);
 
    return 0;
}
/*
void InitDstimg(cv::Mat& dstimg, const cv::Mat& rayvector, const cv::Mat& srcimg)
{
    const int srcWidth = srcimg.cols;
    const int srcHeight = srcimg.rows;
    const int channels = srcimg.channels();
    for(int dstHIdx = 0; dstHIdx < scHeight; dstHIdx++){
        for(int dstWIdx = 0; dstWIdx < scWidth; dstWIdx++){
        
            int dstIdx = dstHIdx * scWidth * channels + dstWIdx * channels; 
            // float x = rayvector.data[dstIdx];
            // float y = rayvector.data[dstIdx + 1];
            // float z = rayvector.data[dstIdx + 2];
            float x = rayvector.at<cv::Vec3f>(dstHIdx,dstWIdx)[0];
            float y = rayvector.at<cv::Vec3f>(dstHIdx,dstWIdx)[1];
            float z = rayvector.at<cv::Vec3f>(dstHIdx,dstWIdx)[2];
            //radian
            float theta = std::acos(y);
            float phi = x != 0 ? std::atan2(x,z) : 0.0f;
            
            int srcWIdx = (int)((pi + phi) / dpi * (srcWidth - 1));
            int srcHIdx = (int)( theta / pi * (srcHeight - 1));
            int srcIdx = srcHIdx * srcWidth * channels + srcWIdx * channels;

            dstimg.data[dstIdx] = srcimg.data[srcIdx];
            dstimg.data[dstIdx + 1] = srcimg.data[srcIdx + 1];
            dstimg.data[dstIdx + 2] = srcimg.data[srcIdx + 2];
            // dstimg.at<Pixel>(dstHIdx,dstWIdx) = srcimg.at<Pixel>(srcHIdx,srcWIdx);
            // dstimg.at<cv::Vec3b>(dsthIdx,dstwIdx)[0] = srcimg.at<cv::Vec3b>(srchIdx,srcwIdx)[0];
            // dstimg.at<cv::Vec3b>(dsthIdx,dstwIdx)[1] = srcimg.at<cv::Vec3b>(srchIdx,srcwIdx)[1];
            // dstimg.at<cv::Vec3b>(dsthIdx,dstwIdx)[2] = srcimg.at<cv::Vec3b>(srchIdx,srcwIdx)[2];
        }
    }
}
*/