#pragma once
#include <opencv2/opencv.hpp>
#include <cmath>


typedef cv::Point3_<uint8_t> Pixel;

//全方位画像から画面に表示する画像を切り出すためのクラス
class Image{
private:
    float mFovy;

    const float maxfovy = 90.0f;
    const float minfovy = 25.0f;

    const double pi = acos(-1);
    const double dpi = pi * 2;

public:
    cv::Mat src; //動画などから読み込んだ全方位画像
    cv::Mat dst; //全方位画像から切り出し画面に映す画像

    Image()
    {
        mFovy = 60.0f;
    }

    //dstの大きさを設定する
    void SetDstMat(cv::Size scSize)
    {
        dst = cv::Mat(scSize, src.type(), cv::Scalar::all(0));
    }

    //視野角を変える
    void Zoom(float angle)
    {
        mFovy += angle;
        mFovy = std::clamp(mFovy, minfovy, maxfovy);
    }

    //元の全方位画像から普通の透視投影の画像を作る
    void MakeDstimg(cv::Vec3d& xAxis, cv::Vec3d& yAxis, cv::Vec3d& zAxis)
    {
    const int srcWidth = src.cols;
    const int srcHeight = src.rows;
    const int channels = src.channels();

    const int dstWidth = dst.cols;
    const int dstHeight = dst.rows;

    cv::Vec3f lefttop, righttop, leftbottom, rightbottom;
    float aspect = (float)dstWidth / (float)dstHeight;
    float correctionX = std::sin((aspect * mFovy / 2.0 * pi) / 180.0);
    float correctionY = std::sin((mFovy / 2.0 * pi) / 180.0);

    //軸を使って画面に投影するスクリーンのワールド座標を計算する
    lefttop = -xAxis * correctionX + yAxis * correctionY + zAxis;
    righttop = xAxis * correctionX + yAxis * correctionY + zAxis;
    leftbottom = -xAxis * correctionX - yAxis * correctionY + zAxis;
    rightbottom = xAxis * correctionX - yAxis * correctionY + zAxis;

    //スクリーンの各ピクセルが円筒座標で何であるかを計算して、
    //全方位画像の対応するピクセルの位置を求めて色を塗る
    dst.forEach<Pixel>
    ([this,&lefttop, &leftbottom, &righttop, &rightbottom,
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
        pixel.x = src.data[srcIdx];
        pixel.y = src.data[srcIdx + 1];
        pixel.z = src.data[srcIdx + 2];
    });
    }
};