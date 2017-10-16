//
// Created by lenovo on 2017/10/9.
//

#include <jni.h>
#include <string>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <unistd.h>
#include "Log.h"
#include "HVideo.h"

void process(const char *inpath);

extern "C" {
#include <libavformat/avformat.h>
ANativeWindow *window = 0;
pthread_t p_tid;
const char *inpath;
int isPlay = 0;
}
HAudio *hAudio;
HVideo *hVideo;

void call_video_play(AVFrame *frame) {
    if (!window) {
        return;
    }
    ANativeWindow_Buffer window_buffer;
    if (ANativeWindow_lock(window, &window_buffer, 0)) {
        return;
    }

    LOGE("绘制 宽%d,高%d", frame->width, frame->height);
    LOGE("绘制 宽%d,高%d  行字节 %d ", window_buffer.width, window_buffer.height, frame->linesize[0]);
    uint8_t *dst = (uint8_t *) window_buffer.bits;
    int dstStride = window_buffer.stride * 4;
    uint8_t *src = frame->data[0];
    int srcStride = frame->linesize[0];
    for (int i = 0; i < window_buffer.height; ++i) {
        memcpy(dst + i * dstStride, src + i * srcStride, srcStride);
    }
    usleep(1000 * 16);
    ANativeWindow_unlockAndPost(window);
}

void *process(void *args) {
    av_register_all();
    avformat_network_init();//网络流需要调用该方法；
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    if (avformat_open_input(&pFormatCtx, inpath, NULL, NULL) != 0) {
        LOGE("视频流打开失败")
        return 0;
    }
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        LOGE("获取流数据失败")
        return 0;
    }
    LOGE("pFormatCtx->nb_streams  %d", pFormatCtx->nb_streams);
    for (int i = 0; i < pFormatCtx->nb_streams; i++) {
        LOGE("pFormatCtx->nb_streams  %d -- %d",i,pFormatCtx->streams[i]->codec->codec_type);
        AVCodecContext *pCodeCtx = pFormatCtx->streams[i]->codec;
        AVCodec *pcodec = avcodec_find_decoder(pCodeCtx->codec_id);
        LOGE("pcodec  %p", pcodec);
        //拷贝一个AVCodecContext
        AVCodecContext *pCodeCtx_ = avcodec_alloc_context3(pcodec);
        avcodec_copy_context(pCodeCtx_, pCodeCtx);
        if (avcodec_open2(pCodeCtx_, pcodec, NULL) < 0) {
            LOGE("%s", "解码器无法打开");
            continue;
        }
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            hVideo->setAvCodecContext(pCodeCtx_);
            hVideo->index = i;
            if (window)
                ANativeWindow_setBuffersGeometry(window, hVideo->codec->width,
                                                hVideo->codec->height,
                                                 WINDOW_FORMAT_RGBA_8888);
            LOGE("解码器打开 视频")
        } else if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            hAudio->setAvCodecContext(pCodeCtx_);
            hAudio->index= i;
            LOGE("解码器打开 音频")

        }

    }
    hVideo->setHAudio(hAudio);
    hVideo->play();
    hAudio->play();

    isPlay = 1;
    //编码数据
    AVPacket *avPacket = (AVPacket *) av_malloc(sizeof(avPacket));
    int ret;
    while (isPlay) {
        ret = av_read_frame(pFormatCtx, avPacket);
        LOGE("编码数据 ret %d",ret);
        if (ret == 0) {
            if (hVideo && hVideo->isPlay && avPacket->stream_index == hVideo->index) {
                LOGE("压入视频");
                hVideo->put(avPacket);
            } else if (hAudio && hAudio->isPlay && avPacket->stream_index == hAudio->index) {
                LOGE("压入音频");
                hAudio->put(avPacket);
            }
            av_packet_unref(avPacket);
        } else if (ret == AVERROR_EOF) {
            //数据读完了
            LOGE("数据读完了");
            while (isPlay) {
                if (hVideo->queue.empty() && hAudio->queue.empty()) {
                    break;
                }
                //  LOGI("等待播放完成");
                av_usleep(10000);
            }
        }
    }
    //    视频解码完     可能视频播放完了   也可能视频没播放完成
    isPlay = 0;
    if (hVideo && hVideo->isPlay) {
        hVideo->stop();
    }
    if (hAudio && hAudio->isPlay) {
        hAudio->stop();
    }
    av_free_packet(avPacket);
    avformat_free_context(pFormatCtx);
    pthread_exit(0);
}

extern "C" {
JNIEXPORT void JNICALL
Java_com_hubing_ffmpeg_FFmpegUtils_HPlayer(
        JNIEnv *env,
        jobject obj, /* this */
        jstring inpath_,
        jobject surface) {
    LOGI("进入HPlayer native方法");
    inpath = env->GetStringUTFChars(inpath_, NULL);
    window = ANativeWindow_fromSurface(env, surface);
    hAudio = new HAudio;
    hVideo = new HVideo;
    hVideo->setPlayCall(call_video_play);
    pthread_create(&p_tid, NULL, process, NULL);


}
}
