//
// Created by lenovo on 2017/10/9.
//

#ifndef FFMPEG_LOG_H
#define FFMPEG_LOG_H
#include <android/log.h>
#define TAG "HPlayer"
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,TAG,FORMAT,##__VA_ARGS__);
#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,TAG,FORMAT,##__VA_ARGS__);
#endif //FFMPEG_LOG_H
