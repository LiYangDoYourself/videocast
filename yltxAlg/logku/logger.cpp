#include "logger.h"
#include <unistd.h>
#include <memory.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

int Logger::mDebugLevel;
std::string Logger::mLogDir;
std::mutex Logger::mMutex;

struct DateTime {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t week;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
};

static int GetSystemTime(struct DateTime *dt)
{
    time_t timer;
    struct tm* t_tm;
    struct DateTime dtTmp;
    time(&timer);
    timer+=8*3600;
    t_tm = gmtime(&timer);

    dtTmp.sec = t_tm->tm_sec;
    dtTmp.min = t_tm->tm_min;
    dtTmp.hour = t_tm->tm_hour;
    dtTmp.day = t_tm->tm_mday;
    dtTmp.month = t_tm->tm_mon + 1;
    dtTmp.year = t_tm->tm_year + 1900;

    if(dt)
        memcpy(dt,&dtTmp,sizeof(struct DateTime));

    return 0;
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

void Logger::init()
{
    mMutex.lock();
    mDebugLevel = 0;
    mLogDir = GetExePath()+"logs/";

    if(access(mLogDir.c_str(),F_OK)!=0)
    {
        printf("create file dir \n");
        CreatDirectory(mLogDir.c_str());

    }

    mMutex.unlock();
}

void Logger::setDebugSW(int sw)
{
    mMutex.lock();

    mDebugLevel = sw;

    mMutex.unlock();
}

int Logger::record(const char *fmt,...)
{
    struct DateTime time;
    FILE *fp;

    va_list ap;

    char pathBuf[512];
    char header[128];

    if(mDebugLevel >0)
    {
        return -1;
    }

    mMutex.lock();


    GetSystemTime(&time);

    sprintf(pathBuf,"%s%04d-%02d-%02d.log",mLogDir.c_str(),time.year,time.month,time.day);
    sprintf(header,"%02d:%02d:%02d: ",time.hour,time.min,time.sec);

    fp = fopen((const char *)pathBuf,"a");
    if(fp==NULL)
    {
        va_start(ap,fmt);
        printf(fmt,ap);
        va_end(ap);
    }
    else
    {
        va_start(ap, fmt);
        fprintf(fp, "%s", header);
        vfprintf(fp, fmt, ap);

        fclose(fp);
    }
    mMutex.unlock();
    return 0;
}

void Logger::removeOverstock()
{
    mMutex.lock();
    scanDirRemoveExceedFile(mLogDir.c_str());

    mMutex.unlock();

}

static int CaculateLagTime(struct DateTime *dt,time_t diff)
{
    if(!dt)
        return -1;
    struct tm _tm;
    struct tm *_tm_p;
    time_t vtime;

    _tm.tm_sec = dt->sec;
    _tm.tm_min = dt->min;
    _tm.tm_hour = dt->hour;
    _tm.tm_mday = dt->day;
    _tm.tm_mon = dt->month - 1;
    _tm.tm_year = dt->year - 1900;
    //printf("start today is %4d-%02d-%02d %02d:%02d:%02d\n", _tm.tm_year+1900, _tm.tm_mon+1, _tm.tm_mday, _tm.tm_hour, _tm.tm_min, _tm.tm_sec);

    vtime = mktime(&_tm);

    vtime += diff;

    _tm_p = localtime(&vtime);

    dt->sec = _tm_p->tm_sec;
    dt->min = _tm_p->tm_min;
    dt->hour = _tm_p->tm_hour;
    dt->day = _tm_p->tm_mday;
    dt->month = _tm_p->tm_mon + 1;
    dt->year = _tm_p->tm_year + 1900;
    return 0;
}

void Logger::scanDirRemoveExceedFile(const char *dirPath)
{
    if(dirPath==NULL)
    {
        return;
    }

    // 獲取文件的信息
    struct stat st;
    lstat(dirPath,&st);

    //判斷一個路徑是不是文件
    if(!S_ISDIR(st.st_mode))
    {
        return;
    }

    struct dirent *entry;
    DIR *dir;
    dir = opendir(dirPath);
    if(dir==NULL)
    {
        return;
    }

    int num2=0;
    while(1)
    {   num2++;
        //if(entry=readdir(dir))!=NULL
        entry = readdir(dir);
        if(entry!=NULL)
        {
        if(entry->d_type==DT_DIR)
        {
            //is DIR
            if(strcmp(entry->d_name,".")==0 || strcmp(entry->d_name,"..")==0)
            {
                continue;
            }
        }else if(entry->d_type==DT_REG)
        {

            //is FILE
            bool del = false;
            int year=-1,month=-1,day=-1;
            char suffix[16]={0};

            sscanf(entry->d_name,"%04d-%02d-%02d.%s",&year,&month,&day,suffix);
            if(year <=-1 ||month<=-1 ||day<=-1 || strcmp(suffix,"log")!=0)
            {
                del=true;
            }
            else
            {

                // 60 tian 7 tian
                static int limitSec =7*(24*60*60);
                struct DateTime time;
                int cfile = (year*10000)+(month*100)+day;
                int climit;
                GetSystemTime(&time);
                CaculateLagTime(&time,0-limitSec);
                climit = (time.year*10000)+(time.month*100)+time.day;
                if(cfile<climit)
                {
                    del = true;
                }
                GetSystemTime(&time);
                CaculateLagTime(&time,0+limitSec);
                climit = (time.year*10000)+(time.month*100)+time.day;
                if(cfile >climit)
                {
                    del = true;
                }


            }

            if(del==true)
            {
                char pathBuf[1024];

                if(dirPath[strlen(dirPath)-1]!='/')
                {
                    sprintf(pathBuf,"%s/%s",dirPath,entry->d_name);
                }
                else
                {
                    sprintf(pathBuf,"%s%s",dirPath,entry->d_name);
                }

                //Logger::record("Remove log file %s \n",pathBuf);
                remove(pathBuf);
            }
        }
        }
        else
        {
            break;
        }
    }


}
