#pragma once
#include <opencv2/opencv.hpp>

class Slider{
private:
    cv::Rect2i mBackGroundArea;
    cv::Rect2i mMoveArea;
    cv::Point2i mStartPos;
    cv::Size2i mSize;
    cv::Scalar mBGColor;
    cv::Scalar mColor;
    cv::Scalar mFontColor;

    int mCount, mTotalCount;
    bool mDragged;

    //スライダーのドラッグ判定の大きさを少し大きくする
    const int mCollisionPad = 10;

public:
    Slider(){
        mStartPos = cv::Point2i(0,0);
        mSize = cv::Size2i(0,0);
        mBackGroundArea = cv::Rect2i(0,0,0,0);
        mMoveArea = cv::Rect2i(0,0,0,0);

        mBGColor = cv::Scalar(255,255,255);
        mColor = cv::Scalar(50,255,50);
        mFontColor = cv::Scalar(255,255,255);

        mCount = 0;
        mTotalCount = 0;
        mDragged = false;
    }

    Slider(cv::Point2i pos, cv::Size2i size, int totalcount){
        mStartPos = pos;
        mSize = size;
        mBackGroundArea = cv::Rect2i(mStartPos,mSize);
        mMoveArea = cv::Rect2i(mStartPos.x, mStartPos.y, 0, mSize.height);

        //デフォルトカラーは適当にに設定する
        mBGColor = cv::Scalar(255,255,255);
        mColor = cv::Scalar(50,255,50);
        mFontColor = cv::Scalar(255,255,255);

        mCount = 0;
        mTotalCount = totalcount;
        mDragged = false;
    }

    void Draw(cv::Mat &img, size_t count){
        mCount = count;
        float progressRate = (float)count / (mTotalCount - 1);
        mMoveArea.width = (float)mBackGroundArea.width * progressRate;
        cv::rectangle(img, mBackGroundArea, mBGColor,-1);
        cv::rectangle(img, mMoveArea, mColor, -1);
        cv::putText(img, std::to_string(count), cv::Point(mStartPos.x, mStartPos.y - 10),
                    cv::FONT_HERSHEY_PLAIN, 1.5, mFontColor, 2.0);
    }

    void JudgeDrag(int x, int y){
        if(x >= mStartPos.x - mCollisionPad && x <= mStartPos.x + mSize.width + mCollisionPad &&
           y >= mStartPos.y - mCollisionPad && y <= mStartPos.y + mSize.height + mCollisionPad){
            mDragged = true;
           }
        else{
            mDragged = false;
        }
        return;
    }

    void MouseDrag(int x){
        x -= mStartPos.x;
        if(x <= 0){
            mCount = 0;
        }
        else if( x >= mSize.width){
            mCount = mTotalCount - 1;
        }
        else{
            mCount = (mTotalCount - 1) * x / mSize.width;
        }
    }

    void ReleaseDrag(){
        mDragged = false;
    }

    bool Dragged(){
        return mDragged;
    }

    int GetCount(){
        return mCount;
    }

    void SetBGColor(cv::Scalar color){
        mBGColor = color;
    }

    void SetColor(cv::Scalar color){
        mColor = color;
    }

    void SetFontColor(cv::Scalar color){
        mFontColor = color;
    }

    void SetTotalCount(int totalcount){
        mTotalCount = totalcount;
    }

    void SetShape(cv::Point2i pos, cv::Size2i size){
        mStartPos = pos;
        mSize = size;
        mBackGroundArea = cv::Rect2i(mStartPos,mSize);
        mMoveArea = cv::Rect2i(mStartPos.x, mStartPos.y, 0, mSize.height);
    }
};
