//
// Created by lenovo on 2017/10/9.
//

#ifndef FFMPEG_HAUDIO_H
#define FFMPEG_HAUDIO_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/time.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <assert.h>
#include <libswresample/swresample.h>
};

#include <queue>
#include <pthread.h>
#include "Log.h"
class HAudio {

public:
    HAudio();
    ~HAudio();
    void setAvCodecContext(AVCodecContext *codecContext);
    void play();
    int get(AVPacket *packet);
    int put(AVPacket *packet);
    int createAudioPlayer();
    void stop();

public:
    AVCodecContext *codecContext;
    SwrContext* swrContext;
    uint8_t *out_buffer;//缓冲区数据
    int out_channer_nb;//1通道数据

    int index;
    int isPlay;
    //    处理线程
    pthread_t p_playid;
    //    同步锁
    pthread_mutex_t mutex;
//    条件变量
    pthread_cond_t cond;
    std::queue<AVPacket *> queue;


    //    相对于第一帧时间
    double clock;

    AVRational time_base;

    //opensl el
    SLObjectItf engineObject;
    SLEngineItf engineEngine;
    SLObjectItf outputMixObject; //混淆器
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb;
    SLObjectItf bqPlayerObject;
    //播放接口
    SLPlayItf bqPlayerPlay;
    //缓冲器队列接口
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
    SLVolumeItf bqPlayerVolume;//音量接口
};

#endif //FFMPEG_HAUDIO_H
