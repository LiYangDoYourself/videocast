#include "videoplayer.h"


extern "C" {
//#include "libavcodec/avcodec.h"
//#include "libavformat/avformat.h"
//#include "libavutil/pixfmt.h"
//#include "libavutil/imgutils.h"
//#include "libswscale/swscale.h"

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>

}

#include "uconfig.h"
#include "ucfunc.h"
#include "cstdlib"
#include "unistd.h"
#include <thread>

#include <list>
#include <mutex>

VideoPlayer::RunStatus VideoPlayer::mStRunStatus[1];
static list<cv::Mat> piclist;
static std::mutex pic_mutex_lock;

_static_ VideoPlayer::VideoPlayer(int sn)
{
    mRunStatus = RunStatus::IDLE;
}

VideoPlayer::~VideoPlayer()
{
     prif("VideoPlayer[%d] delete\n", mSn);
}

void VideoPlayer::globalInit()
{
    avformat_network_init();
    av_register_all();
}

// 保存一幀圖像
void VideoPlayer::setImgCallback(VideoPlayer::ImgCallback callback)
{
    mImgCallback = callback;
}

// 設置視頻序號
void VideoPlayer::setSn(int sn)
{
    mSn = sn;
}

// 返回視頻序號
int VideoPlayer::sn(void)
{
    return mSn;
}

//設置url
void VideoPlayer::setUrl(std::string url)
{
    mUrl = url;
}

// 返回url
std::string VideoPlayer::url(void)
{
    return mUrl;
}

// 返回運行狀態
VideoPlayer::RunStatus VideoPlayer::runStatus()
{
    return mRunStatus;
}


// 是否可視化
bool VideoPlayer::isPreview()
{
    return mPreview;
}

// 是否運行
void VideoPlayer::startPlay()
{
    //start();
    this->run();
}

// 是否停止
void VideoPlayer::stopPlay()
{
    mStop = true;
}

void VideoPlayer::restartPlay()
{
    if (runStatus() == RunStatus::IDLE) {
        this->run();
    } else {
        mStop = true;

//        mTimer.setSingleShot(false);
//        mTimer.start(1000);
//        connect(&mTimer, &QTimer::timeout, this,
//            [this]()->void {
//                if (runStatus() == RunStatus::IDLE) {
//                    mTimer.stop();
//                    start();
//                }
//            }
//        );
    }
}


int ffmpegIntCallback(void *ths)
{
    VideoPlayer *vp = static_cast<VideoPlayer*>(ths);
    if(vp->runStatus()!=VideoPlayer::RunStatus::RUNNING)
    {
        return 0;
    }

    if(vp->mCallbackCtrl==true)
    {
        if(++vp->mCallbackCnt>800)
        {
            vp->mCallbackCnt=0;
            prif("ffmpeg video %d callback timeout\n",vp->sn());
            return 1;
        }
    }

    return 0;
}

int VideoPlayer::createImgPkt(void *buf, int width, int height,int rgbBufsize,int pts, int dts, VideoPlayer::ImgPkt *pkt)
{
    cv::Mat src(height, width, CV_8UC3);
    memcpy(src.data,(uchar *)buf,rgbBufsize);
    pkt->img = src;
    pkt->type = 0;
    pkt->sn = mSn;
    pkt->pts = pts;
    pkt->dts = dts;
    pkt->width = width;
    pkt->height = height;
    pkt->videoDen = mVideoDen;
    pkt->videoNum = mVideoNum;

    return 0;
}


int VideoPlayer::run()
{
    int ret, gotImg;

    mRunStatus = RunStatus::INIT; // 1
    mStRunStatus[mSn]=RunStatus::INIT; // 播放器 1
    prif("Video %d start run.\n", mSn);

    const char *url = mUrl.c_str();

    int result = 0;
    //av_register_all(); // 注册所有FFmpeg库所支持的文件格式和codec
    AVFormatContext* pFormatCtx=NULL;
    char* filename = "rtsp://admin:yltx8888@192.168.37.210:554/h264/ch1/main/av_stream"; //输入文件名
    //char* filename = "rtmp://58.200.131.2:1935/livetv/cctv6"; //输入文件名
//    char* filename = "/home/ly/Downloads/box.mp4";
    pFormatCtx = avformat_alloc_context();
    pFormatCtx->interrupt_callback.callback = ffmpegIntCallback;
    pFormatCtx->interrupt_callback.opaque = this;


    AVDictionary* options = NULL;
    av_dict_set(&options, "rtsp_transport", "tcp", 0);
    av_dict_set(&options, "stimeout", "2000000", 0);//如果没有设置stimeout，那么流地址错误，av_read_frame会阻塞（时间单位是微妙）


    result = avformat_open_input(&pFormatCtx, url, NULL, &options);


    // step1: 打开媒体文件,最后2个参数是用来指定文件格式，buffer大小和格式参数，设置成NULL的话，libavformat库会自动去探测它们

    if (result != 0)
    {
        cout << "open file fail" << endl;
        return -1;
    }
    cout << "open file succ" << endl;

    // step2: 查找信息流的信息
    result = avformat_find_stream_info(pFormatCtx, NULL);
    if (result != 0)
    {
        cout << "find stream fail" << endl;
        return -1;
    }
    cout << "find stream succ" << endl;

    // step3: 打印信息
    av_dump_format(pFormatCtx, 0, filename, 0);

    // step4：找到video流数据
    int i = 0;
    int videoStream = -1;
    AVCodecContext* pCodecCtx = NULL;
    for (i = 0; i < pFormatCtx->nb_streams; i++)
    {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = i;
//            break;
        }
    }

    if (videoStream == -1)
    {
        cout << "find stream video fail" << endl;
        return -1;
    }
    cout << "find stream video succ." << endl;

    cout<<"************"<<endl;
    // 得到video编码格式
//    pCodecCtx = pFormatCtx->streams[videoStream]->codec;
    AVStream *pStream;

    pStream = pFormatCtx->streams[videoStream];
    pCodecCtx = pFormatCtx->streams[videoStream]->codec;
    mVideoDen = pStream->avg_frame_rate.den;
    mVideoNum = pStream->avg_frame_rate.num;

    pCodecCtx->bit_rate = 0;        //初始化为0
    pCodecCtx->time_base.num = 1;   //下面两行：一秒钟25帧
    pCodecCtx->time_base.den = 25;
    pCodecCtx->frame_number = 1;    //每包一个视频帧


    cout<<"111111111"<<endl;

//    int frame_rate = pFormatCtx->streams[videoStream]->avg_frame_rate.num / pFormatCtx->streams[videoStream]->avg_frame_rate.den;//每秒多少
//    cout<<"************* frame_rate="<<frame_rate<<endl;

    // step5: 得到解码器
    AVCodec* pCodec = NULL;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL)
    {
        cout << "find decoder fail" << endl;
        return -1;
    }
    cout << "find decoder succ" << endl;

    result = avcodec_open2(pCodecCtx, pCodec, NULL);
    if (result != 0)
    {
        cout << "open codec fail" << endl;
        return -1;
    }
    cout << "open codec succ" << endl;

    // step6: 申请原始数据帧 和 RGB帧内存
    AVFrame* pFrame = NULL;
    AVFrame* pFrameRGB = NULL;
    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();
    if (pFrame == NULL || pFrameRGB == NULL)
    {
        return -1;
    }

    int numBytes = avpicture_get_size(AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height);
    uint8_t* buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));
    avpicture_fill((AVPicture*)pFrameRGB, buffer, AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height);

    int frameFinishsed = 0;
    AVPacket packet;
    i = 0;

    // step7: 创建格式转化文本
    SwsContext * pSwxCtx = sws_getContext(
        pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
        pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_BGR24,
        SWS_BILINEAR, 0, 0, 0);

    mRunStatus = RunStatus::RUNNING;
    mStRunStatus[mSn] = RunStatus::RUNNING;

    while(true)
    {
        mCallbackCtrl = true;

        // 得到数据包
        result = av_read_frame(pFormatCtx, &packet);
        if (result != 0)
        {
            prif("Av_read_frame video %d read break(ret %d)\n", mSn, ret);
            break;
        }

        mCallbackCtrl= false;
        mCallbackCnt = 0;

        if(mStop==true)
        {
            mStop = false;
            av_free_packet(&packet);
            memset(buffer,0,numBytes);
            struct ImgPkt imgPkt;
            createImgPkt(buffer,pCodecCtx->width,pCodecCtx->height,numBytes,packet.dts,packet.pts,&imgPkt);
            if(mImgCallback) mImgCallback(&imgPkt);
            break;
        }

        if (packet.stream_index == videoStream)
        {
            // 解码
            avcodec_decode_video2(pCodecCtx, pFrame, &frameFinishsed, &packet);

            if (frameFinishsed)
            {
                // 转换
                sws_scale(pSwxCtx, pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
                    pFrameRGB->data, pFrameRGB->linesize);

                struct ImgPkt imgPkt;
                createImgPkt(buffer,pCodecCtx->width,pCodecCtx->height,numBytes,packet.dts,packet.pts,&imgPkt);
                if(mImgCallback) mImgCallback(&imgPkt);

                cv::Mat image(pCodecCtx->height, pCodecCtx->width, CV_8UC3);
                memcpy(image.data,pFrameRGB->data[0],numBytes);
                Size s=Size(image.cols/2,image.rows/2);
                pic_mutex_lock.lock();
                if(piclist.size()<300)
                {
                    if(!image.empty())
                    {
                        //縮放了2倍
                        cv::resize(image,image,s);
                        piclist.push_back(image);
                    }
                }
                pic_mutex_lock.unlock();
//                cv::imshow("pic", image);
//                cv::waitKey(1000/30);
            }
        }
        av_free_packet(&packet);
    }

    av_free(buffer);
    av_free(pFrameRGB);
    av_free(pFrame);
    sws_freeContext(pSwxCtx);
    avcodec_close(pCodecCtx);

proerr_1:
     avformat_close_input(&pFormatCtx);
proeer_0:
     mCallbackCtrl = false;
     mCallbackCnt = 0;
     mRunStatus = RunStatus::IDLE;
     mStRunStatus[mSn] = RunStatus::IDLE;
}

// 把image放到list中
void start_put_frame(VideoPlayer &sp)
{
    sp.run();
}

// 獲取list中的image，並顯示
void start_show_frame()
{
    while(true)
    {
        cv::Mat image;

        pic_mutex_lock.lock();
        if(piclist.size()>0)
        {
            image = piclist.front();
            piclist.pop_front();
        }
        pic_mutex_lock.unlock();

        if(!image.empty())
        {
            cv::imshow("123",image);
            cv::waitKey(1);
        }
    }
}

// test用列
void test_video()
{
    VideoPlayer instance(0);
    instance.globalInit();
    instance.setSn(0);
    instance.setUrl("rtsp://admin:yltx8888@192.168.37.210:554/h264/ch1/main/av_stream");
    //instance.run2();

    VideoPlayer instance2(0);
    instance2.setSn(0);
    instance2.setUrl("rtmp://58.200.131.2:1935/livetv/cctv6");

    thread t1(start_put_frame,std::ref(instance));
//    thread t3(start_put_frame,std::ref(instance2));
    thread t2(start_show_frame);


    t1.join();
//  t3.join();
    t2.join();
}

