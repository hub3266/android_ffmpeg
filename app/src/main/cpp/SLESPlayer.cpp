//
// Created by lenovo on 2017/9/27.
//
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "ffmpegAudio.h"
#include <assert.h>

extern "C" {
//// 创建Audio 结构体
SLObjectItf engineObject = NULL;
//音频引擎
SLEngineItf engineEngine;
//混音器
SLObjectItf outputMixObject = NULL;
// buffer queue player interfaces
SLObjectItf bqPlayerObject = NULL;

SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
const SLEnvironmentalReverbSettings settings = SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT;

//缓冲器队列接口
SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;

//播放接口
SLPlayItf bqPlayerPlay;
SLVolumeItf bqPlayerVolume;

size_t bufferSize;
void *buffer;
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    LOGE("进入bqPlayerCallback");
    bufferSize = 0;
    //assert(NULL == context);
    getPCM(&buffer, &bufferSize);
    LOGE("getPCM");
    // for streaming playback, replace this test by logic to find and fill the next buffer
    if (NULL != buffer && 0 != bufferSize) {
        LOGE("buffer");
        SLresult result;
        // enqueue another buffer
        result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, buffer,
                                                 bufferSize);
        // the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
        // which for this code example would indicate a programming error
        assert(SL_RESULT_SUCCESS == result);
        LOGE(" bqPlayerCallback :%d", result);
    }
}

JNIEXPORT void JNICALL
Java_com_hubing_ffmpeg_FFmpegUtils_openSLELMP3Player(JNIEnv *env, jobject jobj, jstring in_) {
    const char *in = env->GetStringUTFChars(in_, NULL);
    int rate, channel;
    createffmpeg(in,&rate,&channel);

    SLresult sLresult;
    //创建音频引擎
    slCreateEngine(&engineObject,0,NULL,0,NULL,NULL);
    (*engineObject)->Realize(engineObject,SL_BOOLEAN_FALSE);

    (*engineObject)->GetInterface(engineObject,SL_IID_ENGINE,&engineEngine);
    //用音频引擎调用函数 创建混音器outputMixObject
    (*engineEngine)->CreateOutputMix(engineEngine,&outputMixObject,0,0,0);
    // 实现混音器outputMixObject
    (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    // 获取混音器接口outputMixEnvironmentalReverb
    sLresult =(*outputMixObject)->GetInterface(outputMixObject,SL_IID_ENVIRONMENTALREVERB,&outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == sLresult) {
        (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &settings);
    }

    //  配置信息设置
    SLDataLocator_AndroidSimpleBufferQueue android_queue={SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};
    SLDataFormat_PCM pcm={SL_DATAFORMAT_PCM,2,SL_SAMPLINGRATE_44_1,SL_PCMSAMPLEFORMAT_FIXED_16
            ,SL_PCMSAMPLEFORMAT_FIXED_16,SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,SL_BYTEORDER_LITTLEENDIAN};

    //   新建一个数据源 将上述配置信息放到这个数据源中
    SLDataSource slDataSource = {&android_queue, &pcm};
//    设置混音器
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&outputMix, NULL};

    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND,
            /*SL_IID_MUTESOLO,*/ SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
            /*SL_BOOLEAN_TRUE,*/ SL_BOOLEAN_TRUE};
    int reslut=SL_RESULT_SUCCESS==sLresult;

    //先讲这个
    sLresult = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &slDataSource, &audioSnk, 3,
                                                  ids, req);
    LOGE("初始化播放器%d  是否成功 %d   bqPlayerObject  %d",sLresult,reslut,bqPlayerObject);
    //初始化播放器
    (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);

    //    得到接口后调用  获取Player接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);

    //    注册回调缓冲区 //获取缓冲队列接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
                                    &bqPlayerBufferQueue);
    LOGE("获取缓冲区数据");

    (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, NULL);
//    获取音量接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME, &bqPlayerVolume);
    LOGE("获取音量接口");
    //    获取播放状态接口
    (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    LOGE("获取播放状态接口");
    bqPlayerCallback(bqPlayerBufferQueue, NULL);
    LOGE("SLES compleate");
}

// shut down the native audio system
void shutdown()
{
    // destroy buffer queue audio player object, and invalidate all associated interfaces
    if (bqPlayerObject != NULL) {
        (*bqPlayerObject)->Destroy(bqPlayerObject);
        bqPlayerObject = NULL;
        bqPlayerPlay = NULL;
        bqPlayerBufferQueue = NULL;
        bqPlayerVolume = NULL;
    }

    // destroy output mix object, and invalidate all associated interfaces
    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentalReverb = NULL;
    }
    // destroy engine object, and invalidate all associated interfaces
    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }
    // 释放FFmpeg解码器相关资源
    release();
}
}