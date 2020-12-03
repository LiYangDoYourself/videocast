#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <iostream>
#include <memory>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <functional>

using namespace cv;
using namespace std;

void test_video();

class VideoPlayer
{


public:
    enum class RunStatus : int {
        IDLE = 0,
        INIT = 1,
        RUNNING = 2,
        PRESTOP = 3,
    };

    struct ImgPkt {
        int sn;
        int pts;
        int dts;
        int width;
        int height;
        int videoNum;
        int videoDen;
        int type;
        //std::shared_ptr<cv::Mat> img;
        cv::Mat img;
    };

    using ImgCallback = std::function<int(const ImgPkt*)>;


public:
    explicit VideoPlayer(int sn = 0);
    ~VideoPlayer();
    static void globalInit();
    friend int ffmpegIntCallback(void *ths);

    void setImgCallback(ImgCallback callback);
    void setSn(int sn);
    int sn();
    void setUrl(std::string url);
    std::string url();
    RunStatus runStatus();
    bool isPreview();
    void startPlay();
    void stopPlay();
    void restartPlay();
    void startPreview();
    void stopPreview();
    static RunStatus runStatus(int sn);
    int run();

private:
    int createImgPkt(void *buf, int width, int height,int rgbBufsize,int pts, int dts, VideoPlayer::ImgPkt *pkt);



private:
    int mSn;
    std::string mUrl;
    RunStatus mRunStatus;
    bool mPreview;
    bool mStop = false;
    bool mCallbackCtrl = false;
    int mCallbackCnt = 0;

    ImgCallback mImgCallback = nullptr;
    int mVideoDen = 0;
    int mVideoNum = 0;
    static RunStatus mStRunStatus[1];

};

#endif // VIDEOPLAYER_H
