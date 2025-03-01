#pragma once
#include <opencv2/opencv.hpp>

//投影するスクリーンを作るための軸クラス
//軸を回転させることでスクリーンを回転させる
class Axis{
private:
    //回転行列
    cv::Matx33d xRotateMat, yRotateMat, zRotateMat, zyxRotateMat;

public:
    cv::Vec3d x, y, z;

    Axis()
    {
        Reset();
    }

    //視線方向を最初に戻す
    void Reset()
    {
        x = cv::Vec3d(1.0,0.0,0.0);
        y = cv::Vec3d(0.0,1.0,0.0);
        z = cv::Vec3d(0.0,0.0,1.0);
    }

    void Rotate(float roll, float pitch, float yaw)
    {
        cv::Rodrigues(x * roll, xRotateMat);
        cv::Rodrigues(y * pitch,yRotateMat);
        cv::Rodrigues(z * yaw,  zRotateMat);

        zyxRotateMat = xRotateMat * yRotateMat * zRotateMat;
        cv::Matx31d zAxisMat(z);
        cv::Matx31d xAxisMat(x);
        zAxisMat = zyxRotateMat * zAxisMat;
        xAxisMat = zyxRotateMat * xAxisMat;

        z = cv::Vec3d(zAxisMat.val);
        x = cv::Vec3d(xAxisMat.val);

        z = cv::normalize(z);
        x = cv::normalize(x);
        y = z.cross(x);
        y = cv::normalize(y);
    }

    void DrawAxis(cv::Mat &img, cv::Point pos)
    {
        cv::line(img, pos, cv::Point2d(pos.x + x.val[0]*20, pos.y + x.val[1]*20), cv::Scalar(0,0,255),3);
        cv::line(img, pos, cv::Point2d(pos.x + y.val[0]*20, pos.y + y.val[1]*20), cv::Scalar(255,0,0),3);
        cv::line(img, pos, cv::Point2d(pos.x + z.val[0]*20, pos.y + z.val[1]*20), cv::Scalar(0,255,0),3);
        //zの値が大きい順に描画する
    }
};