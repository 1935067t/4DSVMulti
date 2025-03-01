#pragma once
#include <opencv2/opencv.hpp>
#include <string>

class Video{
private:
    bool mPlaying;
    bool mReachEnd;
    int mFramecount;
    int mCurrentframe;
    float mFrameInterval;
    float mDeltaTime;
    cv::TickMeter mTick;
    cv::VideoCapture mCapture;

public:

    void Init(std::string filepath, cv::Mat& srcimg)
    {
        mCapture.open(filepath);
        mFramecount = mCapture.get(cv::CAP_PROP_FRAME_COUNT);
        float framerate = mCapture.get(cv::CAP_PROP_FPS);
        mFrameInterval = 1000.0f / framerate;
        mCapture.read(srcimg);

        mCurrentframe = 0;
        mDeltaTime = 0.0f;
        mPlaying = false;
        if(mCurrentframe == mFramecount - 1) mReachEnd = false;
    }

    //再生・停止切替
    void SwitchPlaying()
    {
        mPlaying = !mPlaying;
        if(mPlaying == true){
            mTick.start();
        }
        else{
            mTick.stop();
            mTick.reset();
        }
        mDeltaTime = 0.0f;
    }

    void Stop()
    {
        mPlaying = false;
    }

    //最後のフレームで無ければ次のフレームを読み込む
    void StepForward(cv::Mat& img)
    {
        if(mReachEnd){
            mPlaying = false;
            return;
        }
        mCapture.read(img);
        mCurrentframe++;
        if(mCurrentframe >= mFramecount - 1){
            mReachEnd = true;
        }
        return;
    }

    //動画再生中に呼び出して画像を読み込んだりする
    //戻り値はキー入力
    int Process(cv::Mat& img)
    {
        mTick.stop();
        mDeltaTime += mTick.getTimeMilli();
        mTick.reset();
        mTick.start();

        int key = cv::pollKey();
    
        if(mDeltaTime >= mFrameInterval){
            mDeltaTime -= mFrameInterval;
            StepForward(img);
        }
        return key;
    }

    void SeekFrame(int frame, cv::Mat&img)
    {
        mPlaying = false;
        mReachEnd = false;
        mCurrentframe = frame;
        if(mCurrentframe < 0){
            mCurrentframe = 0;
        }
        else if(mCurrentframe >= mFramecount - 1){
            mCurrentframe = mFramecount - 1;
            mReachEnd = true;
        }
        mCapture.set(cv::CAP_PROP_POS_FRAMES, mCurrentframe);
        mCapture.read(img);
    }

    int FrameCount(){return mFramecount;}
    int CurrentFrame(){return mCurrentframe;}
    bool Playing(){return mPlaying;}

    //4DSV用
    //動画の入れ替えから現在のフレームの読み込みまで
    void SwitchFile(std::string filepath, cv::Mat& img)
    {
        mCapture.open(filepath);
        mCapture.set(cv::CAP_PROP_POS_FRAMES, mCurrentframe);
        mCapture.read(img);
    }
};