#pragma once
#include <opencv2/opencv.hpp>
#include <string>

class Video{
private:
    bool playing;
    bool reachEnd;
    int framecount;
    int currentframe;
    float frameInterval;
    float deltaTime;
    cv::TickMeter tick;
    cv::VideoCapture capture;

public:

    void Init(std::string filepath, cv::Mat& srcimg)
    {
        capture.open(filepath);
        framecount = capture.get(cv::CAP_PROP_FRAME_COUNT);
        float framerate = capture.get(cv::CAP_PROP_FPS);
        frameInterval = 1000.0f / framerate;
        capture.read(srcimg);

        currentframe = 0;
        deltaTime = 0.0f;
        playing = false;
        if(currentframe == framecount - 1) reachEnd = false;
    }

    //再生・停止切替
    void SwitchPlaying()
    {
        playing = !playing;
        if(playing == true){
            tick.start();
        }
        else{
            tick.stop();
            tick.reset();
        }
        deltaTime = 0.0f;
    }

    void Stop()
    {
        playing = false;
    }

    //最後のフレームで無ければ次のフレームを読み込む
    void StepForward(cv::Mat& img)
    {
        if(reachEnd){
            playing = false;
            return;
        }
        capture.read(img);
        currentframe++;
        if(currentframe >= framecount - 1){
            reachEnd = true;
        }
        return;
    }

    //動画再生中に呼び出して画像を読み込んだりする
    //戻り値はキー入力
    int Process(cv::Mat& img)
    {
        tick.stop();
        deltaTime += tick.getTimeMilli();
        tick.reset();
        tick.start();

        int key = cv::pollKey();
    
        if(deltaTime >= frameInterval){
            deltaTime -= frameInterval;
            StepForward(img);
        }
        return key;
    }

    void SeekFrame(int frame, cv::Mat&img)
    {
        playing = false;
        reachEnd = false;
        currentframe = frame;
        if(currentframe < 0){
            currentframe = 0;
        }
        else if(currentframe >= framecount - 1){
            currentframe = framecount - 1;
            reachEnd = true;
        }
        capture.set(cv::CAP_PROP_POS_FRAMES, currentframe);
        capture.read(img);
    }

    int FrameCount(){return framecount;}
    int CurrentFrame(){return currentframe;}
    bool Playing(){return playing;}

    //4DSV用
    //動画の入れ替えから現在のフレームの読み込みまで
    void SwitchFile(std::string filepath, cv::Mat& img)
    {
        capture.open(filepath);
        capture.set(cv::CAP_PROP_POS_FRAMES, currentframe);
        capture.read(img);
    }
};