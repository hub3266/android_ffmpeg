//
// Created by lenovo on 2017/10/9.
//

#ifndef FFMPEG_HVIDEO_H
#define FFMPEG_HVIDEO_H

//extern "C" {
//#include <libavcodec/avcodec.h>
//#include <libswscale/swscale.h>
//};
#include "HAudio.h"
class HVideo {

public:
    HVideo();
    ~HVideo();
    void setAvCodecContext(AVCodecContext *codecContext);
    void setHAudio(HAudio *hAudio);
    void setPlayCall(void (*call1)(AVFrame *));
    void play();
    int put(AVPacket *packet);
    int get(AVPacket *packet);
    void stop();
    double  synchronize(AVFrame *frame, double play);

public:
    int index;
    int isPlay;
    HAudio* hAudio;
    //    同步锁
    pthread_mutex_t mutex;
//    条件变量
    pthread_cond_t cond;
    std::queue<AVPacket *> queue;
    //    解码器上下文
    AVCodecContext *codec;

    //    处理线程
    pthread_t p_playid;

    AVRational time_base;
    double  clock;
};
#endif //FFMPEG_HVIDEO_H
