#include <opencv2/opencv.hpp>
#include <cmath>
#include <chrono>

typedef cv::Point3_<uint8_t> Pixel;
typedef cv::Point3_<float> Vector;

constexpr const double pi = 3.141592653589793;
constexpr const double dpi = pi*2;
//視野角
const float fov = 60.0f;
//スクリーンサイズ
int scHeight = 500;
int scWidth = 500;
//回転軸
cv::Matx13f xAxis(0.0f);
cv::Matx13f yAxis(0.2f);
cv::Matx13f zAxis(0.0f);

float sensitivityXY = 0.1f;
float sensitivityZ = 0.1f;
// cv::Mat xAxis(cv::Size(3,1),CV_32F,cv::Scalar::all(0.0f));
// cv::Mat yAxis(cv::Size(3,1),CV_32F,cv::Scalar::all(0.2f));
// cv::Mat zAxis(cv::Size(3,1),CV_32F,cv::Scalar::all(0.0f));


//回転行列
// cv::Mat xRotateMat(cv::Size(3,3),CV_32F,cv::Scalar::all(0.0f));
// cv::Mat yRotateMat(cv::Size(3,3),CV_32F,cv::Scalar::all(0.0f));
// cv::Mat zRotateMat(cv::Size(3,3),CV_32F,cv::Scalar::all(0.0f));
cv::Matx33f xRotateMat(0.0f);
cv::Matx33f yRotateMat(0.0f);
cv::Matx33f zRotateMat(0.0f);

void InitAxis()
{
    xAxis.val[0] = 0.1f;
    xAxis.val[1] = 0.0f;
    xAxis.val[2] = 0.0f;

    yAxis.val[0] = 0.0f;
    yAxis.val[1] = 0.1f;
    yAxis.val[2] = 0.0f;

    zAxis.val[0] = 0.0f;
    zAxis.val[1] = 0.0f;
    zAxis.val[2] = 0.1f;

    xAxis = xAxis * sensitivityXY;
    yAxis = yAxis * sensitivityXY;
    zAxis = zAxis * sensitivityZ;

    std::cout << yAxis << std::endl;
}

void InitRayVector(cv::Mat& rayvector)
{
    struct XYCord{
        float x;
        float y;
    };
    XYCord lefttop, righttop, leftbottom, rightbottom;
    float aspect;

    aspect = (float)scWidth / (float)scHeight;

    lefttop.x = std::sin((aspect * fov / 2.0 * pi) / 180.0) * -1;
    lefttop.y = std::sin((fov / 2.0 * pi) / 180.0);

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

    dstimg.forEach<Pixel>
    ([&rayvector, &srcimg, srcWidth, srcHeight, channels](Pixel& pixel, const int position[]) -> void
    {
        float x = rayvector.at<cv::Vec3f>(position[0],position[1])[0];
        float y = rayvector.at<cv::Vec3f>(position[0],position[1])[1];
        float z = rayvector.at<cv::Vec3f>(position[0],position[1])[2];

        float theta = std::acos(y);
        float phi = x != 0 ? std::atan2(x,z) : 0.0f;
            
        int srcWIdx = (int)((pi + phi) / dpi * (srcWidth - 1));
        int srcHIdx = (int)( theta / pi * (srcHeight - 1));

        int srcIdx = srcHIdx * srcWidth * channels + srcWIdx * channels;
        pixel.x = srcimg.data[srcIdx];
        pixel.y = srcimg.data[srcIdx + 1];
        pixel.z = srcimg.data[srcIdx + 2];
        // pixel = srcimg.at<Pixel>(srcHIdx,srcWIdx);
    });
}

void RotateRayVector(cv::Mat& rayvector, float roll, float pitch, float yaw)
{
    cv::Rodrigues(xAxis,xRotateMat);
    cv::Rodrigues(yAxis,yRotateMat);
    cv::Rodrigues(zAxis,zRotateMat);

    std::cout << yRotateMat(0,0) << std::endl;

    rayvector.forEach<cv::Vec3f>([](cv::Vec3f& vec, const int position[]) -> void
    {
        vec[0] = zRotateMat(0,0) * vec[0] + zRotateMat(1,0) * vec[1] + zRotateMat(2,0) * vec[2];
        vec[1] = zRotateMat(0,1) * vec[0] + zRotateMat(1,1) * vec[1] + zRotateMat(2,1) * vec[2];
        vec[2] = zRotateMat(0,2) * vec[0] + zRotateMat(1,2) * vec[1] + zRotateMat(2,2) * vec[2];
        // vec[0] = yRotateMat(0,0) * vec[0] + yRotateMat(0,1) * vec[1] + yRotateMat(0,2) * vec[2];
        // vec[1] = yRotateMat(1,0) * vec[0] + yRotateMat(1,1) * vec[1] + yRotateMat(1,2) * vec[2];
        // vec[2] = yRotateMat(2,0) * vec[0] + yRotateMat(2,1) * vec[1] + yRotateMat(2,2) * vec[2];
    });
}

int main() {
    cv::Mat img;
    img = cv::imread("../omni_image_er.bmp");

    scHeight = 500;
    scWidth = 500;

    cv::Mat rayvector = cv::Mat(cv::Size(scWidth,scHeight),CV_32FC3,cv::Scalar::all(0.0f));
    cv::Mat dstimg(cv::Size(scWidth, scHeight), img.type(), cv::Scalar::all(0));
    std::cout << img.size << std::endl;
    // auto start = std::chrono::high_resolution_clock::now();
    InitAxis();
    InitRayVector(rayvector);
    auto start = std::chrono::high_resolution_clock::now();
    MakeDstimg(dstimg,rayvector,img);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << "経過時間: " << elapsed.count() << " ms" << std::endl;


 
    std::cout << "channel:" << img.channels() << std::endl;
    std::cout << "col:" << img.cols << std::endl;
    std::cout << "row:" << img.rows << std::endl;

    // cv::imshow("title", img);
    cv::imshow("dst",dstimg);
    cv::waitKey(0);

    // InitAxis();
    for(int i = 0; i < 800; i++){
    RotateRayVector(rayvector,0,0,0);
    MakeDstimg(dstimg,rayvector,img);
    cv::imshow("next",dstimg);
    cv::waitKey(0);
    }

    std::cout << yAxis << std::endl;
 
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