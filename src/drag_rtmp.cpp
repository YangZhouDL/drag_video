#include <drag_rtmp.h>

DragRTMP::DragRTMP(const int fps)
{
    ifmt_ctx = NULL;
    pframe = NULL;
    pCodecCtx = NULL;
    pCodec = NULL;

    ret = 0;
    videoindex = -1;

    m_fps = fps;
}

bool DragRTMP::initContext(const string drag_addr)
{
    // Register
    av_register_all();
    // Network
    avformat_network_init();
    // Input
    ret = avformat_open_input(&ifmt_ctx, drag_addr.data(), 0, 0);
    if (ret != 0)
    {
        printError(ret);
        return false;
    }

    ret = avformat_find_stream_info(ifmt_ctx, 0);
    if (ret < 0)
    {
        printError(ret);
        return false;
    }
    av_dump_format(ifmt_ctx, 0, drag_addr.data(), 0);

    videoindex = av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);

    // Find H.264 Decoder
    pCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!pCodec)
    {
        cout << "Failed to find codec!" << endl;
        return false;
    }

    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (!pCodecCtx)
    {
        cout << "Failed to allocate video codec context!" << endl;
        return false;
    }
    pCodecCtx->time_base = {1, m_fps};
    pCodecCtx->framerate = {m_fps, 1};

    ret = avcodec_open2(pCodecCtx, pCodec, NULL);
    if (ret != 0)
    {
        printError(ret);
        return false;
    }

    pframe = av_frame_alloc();
    if (!pframe)
    {
        cout << "Failed to allocate video frame!" << endl;
        return false;
    }

    h264bsf = av_bitstream_filter_init("h264_mp4toannexb");
    if (!h264bsf)
    {
        cout << "Failed to initialize stream filter context!" << endl;
        return false;
    }

    return true;
}

bool DragRTMP::startDrag()
{
    ret = av_read_frame(ifmt_ctx, &pkt);
    if (ret != 0)
    {
        printError(ret);
        return false;
    }

    if (pkt.stream_index == videoindex)
    {
        av_bitstream_filter_filter(h264bsf, ifmt_ctx->streams[videoindex]->codec, NULL, &pkt.data, &pkt.size,
                                   pkt.data, pkt.size, 0);

        // Decode AVPacket
        if (pkt.size)
        {
            ret = avcodec_send_packet(pCodecCtx, &pkt);
            if (ret != 0)
            {
                printError(ret);
                return false;
            }
            // Get AVframe
            ret = avcodec_receive_frame(pCodecCtx, pframe);
            if (ret != 0)
            {
                printError(ret);
                return false;
            }
            // AVframe to image
            AVFrame2Img(pframe);
        }
    }
    // Free AvPacket
    av_packet_unref(&pkt);
}

void DragRTMP::Yuv420p2Rgb32(const uchar *yuvBuffer_in, const uchar *rgbBuffer_out)
{
    uchar *yuvBuffer = (uchar *)yuvBuffer_in;
    uchar *rgb32Buffer = (uchar *)rgbBuffer_out;

    int channels = 3;

    for (int y = 0; y < m_height; y++)
    {
        for (int x = 0; x < m_width; x++)
        {
            int index = y * m_width + x;

            int indexY = y * m_width + x;
            int indexU = m_width * m_height + y / 2 * m_width / 2 + x / 2;
            int indexV = m_width * m_height + m_width * m_height / 4 + y / 2 * m_width / 2 + x / 2;

            uchar Y = yuvBuffer[indexY];
            uchar U = yuvBuffer[indexU];
            uchar V = yuvBuffer[indexV];

            int R = Y + 1.402 * (V - 128);
            int G = Y - 0.34413 * (U - 128) - 0.71414 * (V - 128);
            int B = Y + 1.772 * (U - 128);
            R = (R < 0) ? 0 : R;
            G = (G < 0) ? 0 : G;
            B = (B < 0) ? 0 : B;
            R = (R > 255) ? 255 : R;
            G = (G > 255) ? 255 : G;
            B = (B > 255) ? 255 : B;

            rgb32Buffer[(y * m_width + x) * channels + 2] = uchar(R);
            rgb32Buffer[(y * m_width + x) * channels + 1] = uchar(G);
            rgb32Buffer[(y * m_width + x) * channels + 0] = uchar(B);
        }
    }
}

void DragRTMP::AVFrame2Img(const AVFrame *pFrame)
{
    m_height = pFrame->height;
    m_width = pFrame->width;
    int channels = 3;
    //输出图像分配内存
    m_image = cv::Mat::zeros(m_height, m_width, CV_8UC3);

    //创建保存yuv数据的buffer
    uchar *pDecodedBuffer = new uchar[m_height * m_width * sizeof(uchar) * channels];

    //从AVFrame中获取yuv420p数据，并保存到buffer
    int i, j, k;
    //拷贝y分量
    for (i = 0; i < m_height; i++)
    {
        memcpy(pDecodedBuffer + m_width * i,
               pFrame->data[0] + pFrame->linesize[0] * i,
               m_width);
    }
    //拷贝u分量
    for (j = 0; j < m_height / 2; j++)
    {
        memcpy(pDecodedBuffer + m_width * i + m_width / 2 * j,
               pFrame->data[1] + pFrame->linesize[1] * j,
               m_width / 2);
    }
    //拷贝v分量
    for (k = 0; k < m_height / 2; k++)
    {
        memcpy(pDecodedBuffer + m_width * i + m_width / 2 * j + m_width / 2 * k,
               pFrame->data[2] + pFrame->linesize[2] * k,
               m_width / 2);
    }

    //将buffer中的yuv420p数据转换为RGB;
    Yuv420p2Rgb32(pDecodedBuffer, m_image.data);
}

void DragRTMP::initRectify(Mat& map1,Mat& map2)
{
    Mat K = (cv::Mat_<double>(3, 3) << 286.1809997558594, 0.0, 421.6383972167969,
             0.0, 286.3576965332031, 403.9013977050781,
             0.0, 0.0, 1.0);
    Mat D = (cv::Mat_<double>(4, 1) << -0.008326118811964989, 0.04620290920138359, -0.04403631016612053, 0.00837636087089777);

    int width = 848, height = 800;
    cv::Size imageSize(width, height);
    cv::fisheye::initUndistortRectifyMap(K, D, cv::Mat(), K, imageSize, CV_16SC2, map1, map2);
}

Mat DragRTMP::getImage()
{
    return m_image;
}

void DragRTMP::printError(int err)
{
    char buffer[1024] = {0};
    av_strerror(err, buffer, sizeof(buffer) - 1);
    cout << buffer << endl;
}

DragRTMP::~DragRTMP()
{
    avformat_close_input(&ifmt_ctx);
    avcodec_free_context(&pCodecCtx);
    av_frame_free(&pframe);
    av_free_packet(&pkt);
    av_bitstream_filter_close(h264bsf);

    if (pCodec)
    {
        delete pCodec;
        pCodec = NULL;
    }
    m_image.release();
}