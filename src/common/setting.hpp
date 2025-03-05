#pragma once
#include <tuple>
#include <string>
#include <opencv2/opencv.hpp>

class Setting{
private:

    //起動直後のmain.cppのsensitivityの値
    float mRotationSpeedXY;
    float mRotationSpeedZ;

    //トラックバーの値を保存するための変数
    int mSensitivityXY;
    int mSensitivityZ;
    int mFramerate;

    bool mIsOpened;

    //ウィンドウの名前
    std::string mWindowName;

    const std::string mTrackberNameSensitivityXY = "sensitivity";
    const std::string mTrackberNameSensitivityZ  = "sensitivity z";
    const std::string mTrackberNameFramerate = "framerate";

    const int mWidth = 350;

public:
    Setting()
    {
        mSensitivityXY = 5;
        mSensitivityZ = 5;
        mFramerate = 25;
        mIsOpened = false;
        mWindowName = "Setting";
    }

    //トラックバーで調整する値の初期値を取得する
    void SetInitialValues(float rotationSpeedXY, float rotationSpeedZ, int framerate)
    {
        mRotationSpeedXY = rotationSpeedXY;
        mRotationSpeedZ = rotationSpeedZ;
        mFramerate = framerate;
    }

    void SetWindowName(std::string windowName)
    {
        mWindowName = windowName;
    }

    //トラックバー付きウィンドウを開く
    //変更した値が反映されるのはウィンドウを閉じた後
    void OpenWindow()
    {
        cv::namedWindow(mWindowName);
        cv::Mat dummy = cv::Mat::zeros(1, mWidth, CV_8UC3);
        cv::imshow(mWindowName, dummy);

        cv::createTrackbar(mTrackberNameSensitivityXY, mWindowName, nullptr,10);
        cv::setTrackbarMin(mTrackberNameSensitivityXY, mWindowName, 1);
        cv::setTrackbarPos(mTrackberNameSensitivityXY, mWindowName, mSensitivityXY);

        cv::createTrackbar(mTrackberNameSensitivityZ, mWindowName, nullptr,10);
        cv::setTrackbarMin(mTrackberNameSensitivityZ, mWindowName, 1);
        cv::setTrackbarPos(mTrackberNameSensitivityZ, mWindowName, mSensitivityZ);

        cv::createTrackbar(mTrackberNameFramerate, mWindowName, nullptr,60);
        cv::setTrackbarMin(mTrackberNameFramerate, mWindowName, 1);
        cv::setTrackbarPos(mTrackberNameFramerate, mWindowName, mFramerate);

        mIsOpened = true;
    }

    //トラックバーを閉じる
    //GetTrackbarValuesで値の更新が可能
    void CloseWindow()
    {
        mSensitivityXY = cv::getTrackbarPos(mTrackberNameSensitivityXY, mWindowName);
        mSensitivityZ = cv::getTrackbarPos(mTrackberNameSensitivityZ, mWindowName);
        mFramerate = cv::getTrackbarPos(mTrackberNameFramerate, mWindowName);

        cv::destroyWindow(mWindowName);

        mIsOpened = false;
    }

    //rotationSpeedXY, rotationSpeedZ, framerateをtupleで取得
    std::tuple<float,float,int> GetTrackbarValues()
    {
        float rotationSpeedXY = mRotationSpeedXY * mSensitivityXY / 5.0f;
        float rotationSpeedZ = mRotationSpeedZ * mSensitivityZ / 5.0f;
        
        return std::forward_as_tuple(rotationSpeedXY, rotationSpeedZ, mFramerate);
    }

    bool isOpened(){return mIsOpened;}
};