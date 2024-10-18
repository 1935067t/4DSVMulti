#pragma once
#include <opencv2/opencv.hpp>

void DrawAxis(cv::Mat &img, cv::Vec3d &xAxis, cv::Vec3d &yAxis, cv::Vec3d& zAxis, cv::Point pos)
{
    cv::line(img, pos, cv::Point2d(pos.x + xAxis.val[0]*20, pos.y + xAxis.val[1]*20), cv::Scalar(0,0,255),3);
    cv::line(img, pos, cv::Point2d(pos.x + yAxis.val[0]*20, pos.y + yAxis.val[1]*20), cv::Scalar(255,0,0),3);
    cv::line(img, pos, cv::Point2d(pos.x + zAxis.val[0]*20, pos.y + zAxis.val[1]*20), cv::Scalar(0,255,0),3);
    //zの値が大きい順に描画する
}