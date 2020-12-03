#include <logku/logger.h>
#include <base/videoplayer.h>
#include <base/jsonpara.h>


#if 0
Rect calMaxMinLoc(vector<Point> &pvec)
{
    int minx=10000,miny=10000,maxx=-1,maxy=-1;
    for(auto prect:pvec)
    {
        if(prect.x<minx)
        {
            minx = prect.x;
        }
        if(prect.x>maxx)
        {
            maxx = prect.x;
        }

        if(prect.y<miny)
        {
            miny = prect.y;
        }
        if(prect.y>maxy)
        {
            maxy = prect.y;
        }
    }

    Rect tmp;
    if((maxx-minx)>0)
    {
        tmp = Rect(minx,miny,int(maxx-minx),int(maxy-miny));
    }

    return tmp;
}

void mask_test2()
{
    Mat src = imread("/home/ly/Desktop/2020-04-26-18-42-47-313863.jpg");

    Point root_points[1][6];
    root_points[0][0] = Point(215, 220);
    root_points[0][1] = Point(460, 225);
    root_points[0][2] = Point(466, 450);
    root_points[0][3] = Point(235, 465);
    root_points[0][4] = Point(260, 390);
    root_points[0][5] = Point(96, 310);

    const Point* ppt[1] = { root_points[0] };
    int npt[] = { 6 };
    polylines(src, ppt, npt, 1, 1, Scalar(0,0,0), 1, 8, 0);
    imshow("Test", src);

    cv::Mat mask_ann, dst;
    src.copyTo(mask_ann);
    mask_ann.setTo(cv::Scalar::all(0));

    fillPoly(mask_ann, ppt, npt, 1, Scalar(255, 255, 255));
    imshow("mask_ann", mask_ann);
    src.copyTo(dst, mask_ann);
    imshow("dst", dst);
    waitKey();


}

void mask_test()
{
    Mat src = imread("/home/ly/Desktop/2020-04-26-18-42-47-313863.jpg");
    resize(src,src,Size(src.cols/3,src.rows/3));
    Mat mask_ann,dst;

    src.copyTo(mask_ann);
    mask_ann.setTo(cv::Scalar::all(0));

    vector<Point> listpoint;
    listpoint.push_back(Point(100,100));
    listpoint.push_back(Point(50,300));
    listpoint.push_back(Point(200,100));
    listpoint.push_back(Point(300,200));

    Rect wideRect =calMaxMinLoc(listpoint);

    //    rectangle(src,wideRect,Scalar(255,0,0),2,8);

    int size_n = listpoint.size();
    Point root_points[1][size_n];

    for(int i=0;i<size_n;i++)
    {
        root_points[0][i] = listpoint.at(i);
    }


    const Point* ppt[1] = {root_points[0]};
    int npt[] = {size_n};

    // polylines(src, ppt, npt, 1, 1, Scalar(0,0,0), 2, 8, 0);


    namedWindow("test",1);
    imshow("test",src);

    fillPoly(mask_ann,ppt,npt,1,Scalar(255,255,255));
    imshow("mask_ann",mask_ann);

    src.copyTo(dst,mask_ann);
    imshow("dst",dst);

    waitKey(0);
}

vector<Point> g_movegood;

bool replaceMovearea(vector<Point> &pvec)
{
    bool falg=false;
    if(g_movegood.empty())
    {
        for(auto p:pvec)
        {
            g_movegood.push_back(p);
        }
    }
    else
    {
        if(g_movegood.size()!=pvec.size())
        {
            for(auto p:pvec)
            {
                g_movegood.clear();
                g_movegood.push_back(p);
            }
        }
        else{
            for(int i=0;i<pvec.size();i++)
            {
                if(g_movegood[i].x==pvec[i].x && g_movegood[i].y==pvec[i].y)
                {
                    continue;
                }
                else
                {
                    g_movegood[i]=pvec[i];
                }
            }
        }
    }
}

bool isMove(Mat &src,vector<Point> &pvec)
{




}


string pHashValue(Mat &srcImg2)
{
    Mat srcImg = srcImg2.clone();
    Mat img, dstImg;
    string rst(64, '\0');

    double dIndex[64];
    double mean = 0.0;
    int  k = 0;

    if (srcImg.channels() == 3)//若为彩色图像则转换为灰度图像
    {
        cvtColor(srcImg, srcImg, CV_BGR2GRAY);
        img = Mat_<double>(srcImg);
    }
    else if(srcImg.channels()==4)
    {
        cvtColor(srcImg,srcImg,CV_BGRA2GRAY);
        img = Mat_<double>(srcImg);
    }
    else
    {
        img = Mat_<double>(srcImg);
    }

    //缩放尺寸
    resize(img, img, Size(32, 32));
    //离散余弦变换 DCT
    dct(img, dstImg);

    //获取dct系数均值
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            dIndex[k] = dstImg.at<double>(i, j);
            //计算每个像素的均值
            mean += dstImg.at<double>(i, j) / 64;
            ++k;
        }
    }

    //计算hash
    for (int i = 0; i < 64; ++i)
    {
        if (dIndex[i] >= mean)
        {
            rst[i] = '1';
        }
        else {
            rst[i] = '0';
        }
    }

    return rst;
}

int hanmingDist(string &str1, string &str2)//计算哈希值字符串的汉明距离
{
    if ((str1.size() != 64) || (str2.size() != 64))
    {
        return -1;
    }

    int distValue = 0;
    for (int i = 0; i < 64; i++)
    {
        if (str1[i] != str2[i])
        {
            distValue++;
        }
    }

    return distValue;
}

#include <bitset>
void calchash(Mat_<double> image, const int& height, const int& width, bitset<64>& dhash)
{
    for (int i = 0; i < height; i++)
    {
        int pos = i * height;
        for (int j = 0; j < width - 1; j++)
        {
            dhash[pos + j] = (image(i, j) > image(i, j + 1)) ? 1 : 0;
        }
    }
}

int hammingDistance(const bitset<64>& dhash1, const bitset<64>& dhash2)
{
    int distance = 0;
    for (int i = 0; i < 64; i++) {
        distance += (dhash1[i] == dhash2[i] ? 0 : 1);
    }
    return distance;
}


bool diffpic()
{
    Mat src,dst;
    src = imread("/home/ly/Desktop/differ6.JPG");
    dst = imread("/home/ly/Desktop/differ8.JPG");

    string str1 =pHashValue(src);
    string str2 = pHashValue(dst);

    int dis = hanmingDist(str1,str2);
    cout<<dis<<endl;

    return true;
}


bool diffmat()
{
    //    Mat src,dst;
    //    src = imread("/home/ly/Desktop/differ4.JPG");
    //    dst = imread("/home/ly/Desktop/differ5.JPG");


    Mat img1 = imread("/home/ly/Desktop/differ6.JPG", 1);


    if (!img1.data)
    {
        cout << "the image is not exist!" << endl;
        return 0;
    }

    int rows = 8, cols = 9;
    resize(img1, img1, Size(cols, rows));   // 将图片缩放为8*9
    cvtColor(img1, img1, COLOR_BGR2GRAY);      // 图片进行灰度化

    bitset<64> dhash1;
    calchash(img1, rows, cols, dhash1);

    Mat img2 = imread("/home/ly/Desktop/differ8.JPG");
    if (!img2.data)
    {
        cout << "the image is not exist!" << endl;
        return 0;
    }


    resize(img2, img2, Size(9, 8));   // 将图片缩放为8*9
    cvtColor(img2, img2, COLOR_BGR2GRAY);      // 图片进行灰度化

    bitset<64> dhash2;
    calchash(img2, rows, cols, dhash2);

    int distance = hammingDistance(dhash1, dhash2);      // 计算汉明距离
    cout << distance << endl;;


}


#endif

void start_program()
{
    // 初始化日志
        Logger::init();
    // 刪除7天之前的
        Logger::removeOverstock();
    // set日志等級>0就不記錄了
        Logger::record("12345\n");

    //創建json文件 並內容賦值給mroot mbin
        JsonPara::init();
        JsonPara::loadFile();

    //


    // 讀取視頻並播放
    //test_video();



}



int main()
{
    //mask_test();
    //diffpic();
    //diffmat(); 23
    start_program();

    return 0;
}
