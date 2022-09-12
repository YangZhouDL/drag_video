#ifndef DRAG_RTMP_H
#define DRAG_RTMP_H

#include <iostream>
#include <string>
#include <memory>

extern "C"
{
#include "libavformat/avformat.h"
#include <libavutil/mathematics.h>
#include <libavutil/time.h>
#include <libavutil/samplefmt.h>
#include <libavcodec/avcodec.h>
};

#include "opencv2/core.hpp"
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

class DragRTMP
{
private:
    AVFormatContext *ifmt_ctx;
    AVPacket pkt;
    AVFrame *pframe;
    int ret;
    int videoindex;

    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;

    AVBitStreamFilterContext *h264bsf;

    int m_fps;
    Mat m_image;
    int m_height;
    int m_width;
    Mat m_map1,m_map2;

public:
    typedef shared_ptr<DragRTMP> Ptr;

    DragRTMP(const int fps);

    bool initContext(const string drag_addr);

    bool startDrag();

    void Yuv420p2Rgb32(const uchar *yuvBuffer_in, const uchar *rgbBuffer_out);

    void AVFrame2Img(const AVFrame *pFrame);

    void initRectify();

    void rectifyImg(Mat& img);

    Mat getImage();

    void printError(int err);

    ~DragRTMP();
};

#endif