//
// Created by lenovo on 2017/9/26.
//

#ifndef FFMPEG_FFMPEG_H
#define FFMPEG_FFMPEG_H

#endif //FFMPEG_FFMPEG_H

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include "libswscale/swscale.h"
#include <android/native_window_jni.h>
#include <unistd.h>
#include <android/log.h>
};
#define TAG "FFMPEG"
#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,TAG,FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,TAG,FORMAT,##__VA_ARGS__);
extern "C" {
int createffmpeg(const char *input, int *rate, int *channel);
int getPCM(void **pcm, size_t *pcm_size);
void release();
}
