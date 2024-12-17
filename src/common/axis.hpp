#pragma once
#include <opencv2/opencv.hpp>

class Axis{
private:
    //回転軸
    cv::Vec3d xAxis, yAxis, zAxis;
    //回転行列
    cv::Matx33d xRotateMat, yRotateMat, zRotateMat, zyxRotateMat;

public:
    Axis()
    {
        Reset();
    }

    void Reset()
    {
        xAxis = cv::Vec3d(1.0,0.0,0.0);
        yAxis = cv::Vec3d(0.0,1.0,0.0);
        zAxis = cv::Vec3d(0.0,0.0,1.0);
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
};

void DrawAxis(cv::Mat &img, cv::Vec3d &xAxis, cv::Vec3d &yAxis, cv::Vec3d& zAxis, cv::Point pos)
{
    cv::line(img, pos, cv::Point2d(pos.x + xAxis.val[0]*20, pos.y + xAxis.val[1]*20), cv::Scalar(0,0,255),3);
    cv::line(img, pos, cv::Point2d(pos.x + yAxis.val[0]*20, pos.y + yAxis.val[1]*20), cv::Scalar(255,0,0),3);
    cv::line(img, pos, cv::Point2d(pos.x + zAxis.val[0]*20, pos.y + zAxis.val[1]*20), cv::Scalar(0,255,0),3);
    //zの値が大きい順に描画する
}