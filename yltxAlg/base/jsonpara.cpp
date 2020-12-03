#include "jsonpara.h"
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include "uconfig.h"
#include "logku/logger.h"
//#include "getcurrenttime.h"

_static_ JsonPara::Bin JsonPara::mBin;
_static_ std::string JsonPara::mFilePath;
_static_ std::string JsonPara::mDirPath;
_static_ Json JsonPara::mRoot;
_static_ bool JsonPara::mParaChanged;
_static_ bool JsonPara::mUrlChanged[1];

static void createNewJsonPara(std::string &filePath)
{
    Json root;
    root["deviceID"]="001";
    root["stationID"] = "64";
    root["boardIndex"] = "001";
    root["description"] = "rk3399_device";
    root["serverIP"] = "192.168.37.205";
    root["serverPort"] =6789;

    Json channel;
    channel["groupMinNum"] = 4;
    channel["groupDistanceFactor"] = 6.0;
    channel["rtspUrl"] = "null";
    channel["area"] = "null";
    channel["type"] = "null";
    channel["description"] = "null";
    channel["user"] = "null";
    channel["password"] = "null";
    channel["ip"] = "192.168.1.100";
    channel["port"] = 8888;
    channel["type"] = "unknow";
    channel["preview"] = true;

    channel["sn"] = 0;
    channel["enable"] = true;
    channel["algorithmOnoff"] = {true, false, false, false, false, false, false, false};
    channel["rtspUrl"] = "rtmp://58.200.131.2:1935/livetv/dftv";
    root["channel"].push_back(channel);

    std::ofstream os(filePath);
    std::cout<<"JsonPara store file:"<<filePath<<std::endl<<root.dump(4)<<std::endl;
    os<<root.dump(4);
    os.close();
}

static std::string GetExePath()
{
    int rval;
    char link_target[4096];
    char *last_slash;
    size_t result_Length;
    char *result;
    std::string strExeDir;

    // 可以獲取當前程序的路徑
    rval = readlink("/proc/self/exe",link_target,4096);
    if(rval<0 || rval >=1024)
    {
            return "";
    }
    link_target[rval]='\0';
    last_slash = strrchr(link_target,'/');

    if(last_slash ==0 || last_slash==link_target)
    {
        return "";
    }
    result_Length = last_slash-link_target;
    result = (char*)malloc(result_Length+1);
    strncpy(result,link_target,result_Length);
    result[result_Length] = '\0';

    strExeDir.append(result);
    strExeDir.append("/");
    free(result);

    return strExeDir;
}

static int CreatDirectory(const char *path)
{
    if(!path)
    {
        return -1;
    }
    char dir[512];
    int i,len;
    strcpy(dir,path);
    len = strlen(dir);

    if(dir[len-1]!='/')
        strcat(dir,"/");

    len = strlen(dir);

    for(i=1;i<len;i++)
    {
        if(dir[i]=='/')
        {
           dir[i]=0;
           // not exist
           if(access(dir,F_OK)!=0)
           {
                if(mkdir(dir,S_IRUSR |S_IWUSR |S_IXUSR| S_IRGRP |S_IROTH|S_IXOTH)==-1)
                {
                    return -2;
                }
           }
           else
           {
               printf("%s exist\n",dir);
           }
           dir[i]='/';
        }
    }

    return 0;
}

bool judgeFileExist(const char *filepath)
{
     struct stat buffer;
     return (stat (filepath, &buffer) == 0);
}

// 初始化創建一個json文件
_static_ int JsonPara::init()
{
    mFilePath =GetExePath()+"/para/config.json";
    mDirPath = GetExePath()+"/para";
    mParaChanged = true;

    for(bool &changed:mUrlChanged)
    {
        changed = false;
    }
    return 0;
}
// 導入文件並將 mroot mbin
_static_ int JsonPara::loadFile()
{
    //char *path = static_cast<char *>(mDirPath.c_str());
    if(!judgeFileExist(mDirPath.c_str()))
    {
        CreatDirectory(mDirPath.c_str());
    }

    if(!judgeFileExist(mFilePath.c_str()))
    {
        createNewJsonPara(mFilePath);
    }

    try
    {
        std::ifstream is(mFilePath);
        mRoot = Json::parse(is);
        is.close();
    }catch(Json::exception &e)
    {
        Logger::record("Json para parse failed %s\n",e.what());
        createNewJsonPara(mFilePath);
        std::ifstream is(mFilePath);
        mRoot = Json::parse(is);
        is.close();
    }

    updateBin();
    return 0;
}

//重新存儲一下文件並更新 mroot和mbin
_static_ int JsonPara::storeFile()
{
    std::ofstream os(mFilePath);

    os<<mRoot.dump(4);
    os.close();

    updateBin();
    mParaChanged = true;

    return 0;
}

_static_ Json *JsonPara::root()
{
    return &mRoot;
}

_static_ JsonPara::Bin *JsonPara::bin()
{
    return &mBin;
}

bool JsonPara::paraChanged()
{
    bool ret=mParaChanged;
    mParaChanged = false;
    return ret;
}

bool JsonPara::urlChanged(int ch)
{
    bool ret=mUrlChanged[ch];
    mUrlChanged[ch]=false;
    return ret;
}

_static_ void JsonPara::updateBin()
{
    int ch=0;
    JsonPara::Channel *pChan;
    for(bool &changed:mUrlChanged)
    {
        changed=false;
    }

    if (mRoot["deviceID"].is_string()) mBin.deviceID = mRoot["deviceID"];
    if (mRoot["stationID"].is_string()) mBin.stationID = mRoot["stationID"];
    if (mRoot["boardIndex"].is_string()) mBin.boardIndex = mRoot["boardIndex"];
    if (mRoot["description"].is_string()) mBin.description = mRoot["description"];
    if (mRoot["serverIP"].is_string()) mBin.serverIP = mRoot["serverIP"];
    if (mRoot["serverPort"].is_number()) mBin.serverPort = mRoot["serverPort"];

    if(mRoot["channel"].is_array())
    {
        Json vChannel;
        jtov3(vChannel,(&mRoot),"channel");

        for(Json chan:vChannel)
        {
            pChan = &mBin.channel[ch];
            if (chan["sn"].is_number()) pChan->sn = chan["sn"];
            if (chan["enable"].is_boolean()) pChan->enable = chan["enable"];
            if (chan["preview"].is_boolean()) pChan->preview = chan["preview"];
            if (chan["area"].is_string()) pChan->area = chan["area"];
            if (chan["type"].is_string()) pChan->type = chan["type"];
            if (chan["description"].is_string()) pChan->description = chan["description"];
            if (chan["rtspUrl"].is_string()) {
                if (pChan->rtspUrl.compare(chan["rtspUrl"]) != 0) {
                    pChan->rtspUrl = chan["rtspUrl"];
                    mUrlChanged[ch] = true;
                }
            }

            if (chan["groupMinNum"].is_number()) pChan->groupMinNum = chan["groupMinNum"];
            if (chan["groupDistanceFactor"].is_number_float()) pChan->groupDistanceFactor = chan["groupDistanceFactor"];
            if (chan["groupDistanceFactor"].is_number_float()) pChan->groupDistanceFactor = chan["groupDistanceFactor"];

            Json onoff;
            jtov3(onoff, (&chan), "algorithmOnoff");
            if (chan["algorithmOnoff"].is_array()) {
                if (onoff[0].is_boolean()) pChan->algorithmOnoff[0] = onoff[0];
                if (onoff[1].is_boolean()) pChan->algorithmOnoff[1] = onoff[1];
                if (onoff[2].is_boolean()) pChan->algorithmOnoff[2] = onoff[2];
                if (onoff[3].is_boolean()) pChan->algorithmOnoff[3] = onoff[3];
                if (onoff[4].is_boolean()) pChan->algorithmOnoff[4] = onoff[4];
                if (onoff[5].is_boolean()) pChan->algorithmOnoff[5] = onoff[5];
                if (onoff[6].is_boolean()) pChan->algorithmOnoff[6] = onoff[6];
                if (onoff[7].is_boolean()) pChan->algorithmOnoff[7] = onoff[7];
            }

            for(int i=0;i<GlobalAlgNums_t::MaxAlgNum;++i)
            {
                std::string key = "algArea"+std::to_string(i);
                Json area;
                pChan->ratios[i].clear();
                jtov3(area,(&chan),key);

                if(area.is_array())
                {
                    for(auto &elem:area)
                    {
                        Ratio ratio;
                        if(elem["x"].is_number_float()) ratio.x = elem["x"];
                        if (elem["y"].is_number_float()) ratio.y = elem["y"];
                        pChan->ratios[i].push_back(ratio);
                    }
                }

            }

            if(++ch>=1)
            {
                break;
            }
        }
    }
}

