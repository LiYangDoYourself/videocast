#ifndef JSONPARA_H
#define JSONPARA_H

#include <string>
#include <stdint.h>
#include "third/nlohmannjson.h"

class JsonPara
{
public:
    struct Ratio
    {
        double x=0;
        double y=0;
    };

    struct Channel {
        int sn; // 序號
        bool enable; //是否開啓
        bool preview; //是否預覽
        bool algorithmOnoff[16]; // 開啓那些算法

        std::vector<Ratio> ratios[16]; // 每個算法對應的區域

        float groupDistanceFactor; // 人員距離
        int groupMinNum;  // 人員數量
        std::string rtspUrl; //rtspurl地址
        std::string ip;
        uint16_t port;
        std::string password;
        std::string user;
        std::string type;
        std::string area;
        std::string description;
    };

    struct Bin{
        std::string deviceID;  // 設備ID
        std::string stationID; //站ID
        std::string boardIndex; //
        std::string description; // 描述
        std::string serverIP; // 服務器地址
        uint16_t serverPort; //服務起端口
        Channel channel[1]; // channel 對象數組
    };

public:
    static int init(); //初始化創建一個json文件
    static int loadFile(); //導入文件並將 mroot mbin
    static int storeFile(); // 重新存儲一下文件並更新 mroot和mbin
    static Json *root();
    static JsonPara::Bin *bin();
    static bool paraChanged();
    static bool urlChanged(int ch);

private:
    static void updateBin();


private:
    static JsonPara::Bin mBin;
    static Json mRoot;
    static std::string mFilePath;
    static std::string mDirPath;
    static bool mParaChanged;
    static bool mUrlChanged[1];

private:
    JsonPara()=default;
    ~JsonPara()=default;
};

#endif // JSONPARA_H
/*
 * JsonPara store file:/work/yltx/build-yltxProj-Desktop-Debug/para/config.json
{
    "//": "视频通道数组，当前默认为4通道"
    "channel": [
        {
            "//": "视频通道算法开关: \
                    0聚众检测 \
                    1工服检测 \
                    2区域入侵 \
                    3打电话检测 \
                    4吸烟检测 \
                    5倒地检测 \
                    6斗殴检测"
            "algorithmOnoff": [
                true,
                false,
                false,
                false,
                false,
                false,
                false,
                false
            ],

            "//": "视频通道是否开启"
            "enable": true,

            "//": "视频通道是否开启预览"
            "preview": true,

            "//": "聚众人数距离因子，范围1.0-10.0"
            "groupDistanceFactor": 6.0,

            "//": "聚众人数最小数量，范围2-100"
            "groupMinNum": 4,

            "//": "当前视频通道摄像头编号"
            "sn": 0,

            "//": "视频通道摄像头IP地址"
            "ip": "192.168.1.100",

            "//": "视频通道摄像头端口"
            "port": 8888,

            "//": "视频通道摄像头用户名"
            "user": "null",

            "//": "视频通道摄像头密码"
            "password": "null",

            "//": "视频通道视频类型，或厂家名称"
            "type": "unknow"

            "//": "摄像头位置区域"
            "area": "unknow"

            "//": "摄像头描述信息"
            "description": "unknow"

            "//": "视频通道摄像头RTSP地址，如才用rtsp地址，\
                    则ip，port，user，password，type则可忽略"
            "rtspUrl": "rtmp://58.200.131.2:1935/livetv/dftv",
        },

        "//": "共4组视频通道，以下3组同上"
        ......
    ],

    "//": "主板设备唯一ID号，最大10位"
    "deviceID": "001002",

    "//": "主板设备放置站点的唯一ID号，最大10位"
    "stationID": "000001"

    "//": "主板设备编号，最大10位"
    "boardIndex": "000001"

    "//": "主板描述信息"
    "description": "new device"

    "//": "服务器IP"
    "serverIP": "192.168.1.166"

    "//": "服务器Port"
    "serverPort": 8080

}
*/
