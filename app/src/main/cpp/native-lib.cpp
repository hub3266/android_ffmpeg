#include <jni.h>
#include <string>
#include <android/log.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <android/window.h>
#include <android/native_window_jni.h>
#include <unistd.h>
#include <assert.h>
#include <libswresample/swresample.h>
}
#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"ffmpeg",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"ffmpeg",FORMAT,##__VA_ARGS__);
# define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))

extern "C" {
JNIEXPORT jstring JNICALL
Java_com_hubing_ffmpeg_FFmpegUtils_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "111111111111111111111111111111";
    av_register_all();
    return env->NewStringUTF(hello.c_str());
}

JNIEXPORT void JNICALL mp4_to_yuv(JNIEnv *env, jobject obj, jstring inputStr_, jstring outputStr_) {
    const char *inputStr = env->GetStringUTFChars(inputStr_, NULL);
    const char *outputStr = env->GetStringUTFChars(outputStr_, NULL);
    LOGE("inputStr####%s", inputStr);
    LOGE("outputStr####%s", outputStr);
    //注册各大组件
    av_register_all();
    //获取转码上下文
    AVFormatContext *pAVFormatContext = avformat_alloc_context();
    if (avformat_open_input(&pAVFormatContext, inputStr, NULL, NULL) < 0) {
        LOGE("打开文件失败  ");
        return;
    }
    // pAVFormatContext 填充数据
    if (avformat_find_stream_info(pAVFormatContext, NULL) < 0) {
        LOGE("获取信息失败");
        return;
    }
    int vedio_stream_idx = -1;
    //找到视频流
    for (int i = 0; i < pAVFormatContext->nb_streams; ++i) {
        if (pAVFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            vedio_stream_idx = i;
        }
    }
    LOGE("vedio_stream_idx :%d ", vedio_stream_idx);
    //解码器上下文
    AVCodecContext *pAVCodecContext = pAVFormatContext->streams[vedio_stream_idx]->codec;
    //解码器
    AVCodec *avCodec = avcodec_find_decoder(pAVCodecContext->codec_id);

    if (avcodec_open2(pAVCodecContext, avCodec, NULL) < 0) {
        LOGE("解码失败");
        return;
    }

    //分配内存
    AVPacket *pAVPacket = (AVPacket *) av_malloc(sizeof(AVPacket));
    //初始化 数据
    av_init_packet(pAVPacket);

    AVFrame *pAVFrame = av_frame_alloc();
    AVFrame *pYuvAVFrame = av_frame_alloc();

    //    给yuvframe  的缓冲区 初始化
    uint8_t *buffSize = (uint8_t *) av_malloc(
            avpicture_get_size(AV_PIX_FMT_YUV420P, pAVCodecContext->width,
                               pAVCodecContext->height));
    avpicture_fill((AVPicture *) pYuvAVFrame, buffSize, AV_PIX_FMT_YUV420P, pAVCodecContext->width,
                   pAVCodecContext->height);
    LOGE("宽 %d  高 %d", pAVCodecContext->width, pAVCodecContext->height);

    SwsContext *swsContext = sws_getContext(pAVCodecContext->width, pAVCodecContext->height,
                                            pAVCodecContext->pix_fmt,
                                            pAVCodecContext->width, pAVCodecContext->height,
                                            AV_PIX_FMT_YUV420P,
                                            SWS_BILINEAR, NULL, NULL, NULL);
    FILE *fp_yuv = fopen(outputStr, "wb");

    //packet入参 出参对象  转换上下文
    int got_frame;
    //读取码流中的音频若干帧或者视频一帧
    while (av_read_frame(pAVFormatContext, pAVPacket) >= 0) {
        //解封装
        // 根据frame 进行原生绘制    bitmap  window
        //got_frame 0 :没有frame
        avcodec_decode_video2(pAVCodecContext, pAVFrame, &got_frame, pAVPacket);

        if (got_frame > 0) {
            sws_scale(swsContext, (const uint8_t *const *) pAVFrame->data, pAVFrame->linesize, 0,
                      pAVFrame->height, pYuvAVFrame->data, pYuvAVFrame->linesize);
            int y_size = pAVCodecContext->width * pAVCodecContext->height;

            fwrite(pYuvAVFrame->data[0], 1, y_size, fp_yuv);
            fwrite(pYuvAVFrame->data[1], 1, y_size / 4, fp_yuv);
            fwrite(pYuvAVFrame->data[2], 1, y_size / 4, fp_yuv);
        }

        av_free_packet(pAVPacket);
    }
    fclose(fp_yuv);
    sws_freeContext(swsContext);
    av_frame_free(&pAVFrame);
    av_frame_free(&pYuvAVFrame);
    avcodec_close(pAVCodecContext);
    avcodec_free_context(&pAVCodecContext);
    avformat_free_context(pAVFormatContext);

    env->ReleaseStringUTFChars(inputStr_, inputStr);
    env->ReleaseStringUTFChars(outputStr_, outputStr);
    LOGE("complate");

}

JNIEXPORT void JNICALL player(JNIEnv *env, jobject obj, jstring inpath_, jobject surface) {
    const char *inpath = env->GetStringUTFChars(inpath_, NULL);
    //注册各大组件
    av_register_all();

    AVFormatContext *pAVFormatContext = avformat_alloc_context();

    if (avformat_open_input(&pAVFormatContext, inpath, NULL, NULL) < 0) {
        LOGE("打开文件失败！！")
        return;
    }

    if (avformat_find_stream_info(pAVFormatContext, NULL) < 0) {
        LOGE("获取数据流信息！！")
        return;
    }

    int vedio_stream_idx = -1;
    int i = 0;
    for (int i = 0; i < pAVFormatContext->nb_streams; i++) {
        if (pAVFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            vedio_stream_idx = i;
        }
    }

    //解码器上下文
    AVCodecContext *pAVCodecContext = pAVFormatContext->streams[vedio_stream_idx]->codec;
    //解码器
    AVCodec *pAVCodec = avcodec_find_decoder(pAVCodecContext->codec_id);

    //解码
    if (avcodec_open2(pAVCodecContext, pAVCodec, NULL) < 0) {
        LOGE("解码失败！！")
        return;
    }

    AVPacket *avpacket = (AVPacket *) av_malloc(sizeof(AVPacket));

    AVFrame *src_frame = av_frame_alloc();
    AVFrame *des_frame = av_frame_alloc();
    //    给des_frame  的缓冲区 初始化
    uint8_t *buffSize = (uint8_t *) av_malloc(
            avpicture_get_size(AV_PIX_FMT_RGBA, pAVCodecContext->width,
                               pAVCodecContext->height));
    avpicture_fill((AVPicture *) des_frame, buffSize, AV_PIX_FMT_RGBA, pAVCodecContext->width,
                   pAVCodecContext->height);
    LOGE("宽 %d  高 %d", pAVCodecContext->width, pAVCodecContext->height);

    SwsContext *swsContext = sws_getContext(pAVCodecContext->width, pAVCodecContext->height,
                                            pAVCodecContext->pix_fmt,
                                            pAVCodecContext->width, pAVCodecContext->height,
                                            AV_PIX_FMT_RGBA, SWS_BICUBIC,
                                            NULL, NULL, NULL);
    LOGE("获取转换上下文");

    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    ANativeWindow_Buffer outBuffer;
    //packet入参 出参对象  转换上下文
    int got_frame;
    //读取码流中的音频若干帧或者视频一帧
    while (av_read_frame(pAVFormatContext, avpacket) >= 0) {
        //解封装
        // 根据frame 进行原生绘制    bitmap  window
        //got_frame 0 :没有frame
        avcodec_decode_video2(pAVCodecContext, src_frame, &got_frame, avpacket);
        LOGE("got_frame%d", got_frame);
        if (got_frame) {
            // WINDOW_FORMAT_RGBA_8888          = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
            //ANativeWindow_setBuffersGeometry(nativeWindow, pAVCodecContext->width,
            //                                pAVCodecContext->height, WINDOW_FORMAT_RGBA_8888);
            LOGE("pAVCodecContext->width %d  pAVCodecContext->height %d", pAVCodecContext->width,
                 pAVCodecContext->height);
            float scale = (1200 / (float) pAVCodecContext->width);
            LOGE("1111111111111111111111111111");
            ANativeWindow_setBuffersGeometry(nativeWindow, (int32_t) (1200 / scale),
                                             (int32_t) (1900 / scale), WINDOW_FORMAT_RGBA_8888);
            LOGE("2222222222222222222222");
            int32_t lock = ANativeWindow_lock(nativeWindow, &outBuffer, NULL);
            sws_scale(swsContext, (const uint8_t *const *) src_frame->data, src_frame->linesize,
                      0,
                      pAVCodecContext->height, des_frame->data, des_frame->linesize);
            //rgb 画面所有数据 首地址
            uint8_t *out = (uint8_t *) outBuffer.bits;
            //   每行数据
            //outBuffer.stride  缓冲区一行的pix 数
            //outStride  pix数*4 = bite数
            int32_t outStride = outBuffer.stride * 4;
            // 实际内存  首地址
            uint8_t *des = (uint8_t *) des_frame->data[0];
            // 实际  每行的bite数
            int desStride = des_frame->linesize[0];
            LOGE("outStride %d  desStride %d", outStride, desStride);

            for (int i = 0; i < (pAVCodecContext->height); ++i) {
                memcpy(out + i * outStride, des + i * desStride, desStride);
            }

            if (lock == 0) {
                ANativeWindow_unlockAndPost(nativeWindow);
            }

            usleep(1000 * 16);
        }
        av_free_packet(avpacket);
    }

    //sws_freeContext(swsContext);
    ANativeWindow_release(nativeWindow);
    av_frame_free(&src_frame);
    //av_frame_free(&des_frame);
    avcodec_close(pAVCodecContext);
    //avcodec_free_context(&pAVCodecContext);
    avformat_free_context(pAVFormatContext);
    env->ReleaseStringUTFChars(inpath_, inpath);

}

JNIEXPORT void JNICALL MP3_to_pcm(JNIEnv *env, jobject obj, jstring inputStr_, jobject audioPlayer) {
    const char *inputStr = env->GetStringUTFChars(inputStr_, NULL);
    //const char *outputStr = env->GetStringUTFChars(outputStr_, NULL);
    LOGE("inputStr%s",inputStr)
    //注册各大组件
    av_register_all();


    AVFormatContext *pAVFormatContext = avformat_alloc_context();

    if (avformat_open_input(&pAVFormatContext, inputStr, NULL, NULL) != 0) {
        LOGE("打开文件失败！！")
        return;
    }

    if (avformat_find_stream_info(pAVFormatContext, NULL) < 0) {
        LOGI("%s", "无法获取输入文件信息");
        return;
    }

    int audio_stream_idx = -1;
    for ( int i = 0; i < pAVFormatContext->nb_streams; ++i) {
        if (pAVFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            LOGE("%d  找到音频id  %d", i,pAVFormatContext->streams[i]->codec->codec_type);
            audio_stream_idx = i;
            break;
        }
    }

    AVCodecContext *avCodecContext = pAVFormatContext->streams[audio_stream_idx]->codec;
    LOGE("获取视频编码器上下文 %p  ",avCodecContext);
    AVCodec *avCodec = avcodec_find_decoder(avCodecContext->codec_id);
    //const AVCodec* avCodec = avCodecContext->codec;
    if(avCodec==NULL){
        LOGE("获取视频编码null  avCodec %p",avCodec);
    }
    int ret = avcodec_open2(avCodecContext, avCodec, NULL);
    if (ret < 0) {
       LOGE("%d  %s",ret, "无法打开解码器");
        return;
    }

    AVPacket *avPacket = (AVPacket *) av_malloc(sizeof(AVPacket));
    AVFrame *srcAVFram = av_frame_alloc();
    AVFrame *dstAVFram = av_frame_alloc();

    SwrContext *swrContext = swr_alloc();

    //    音频格式  重采样设置参数
    AVSampleFormat in_sample = avCodecContext->sample_fmt;
//    输出采样格式
    AVSampleFormat out_sample = AV_SAMPLE_FMT_S16;
// 输入采样率
    int in_sample_rate = avCodecContext->sample_rate;
//    输出采样
    int out_sample_rate = 44100;

//    输入声道布局
    uint64_t in_ch_layout = avCodecContext->channel_layout;
//    输出声道布局
    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;
    //    设置音频缓冲区间 16bit   44100  PCM数据
    uint8_t *out_buffer = (uint8_t *) av_malloc(2 * 44100);

    swr_alloc_set_opts(swrContext,out_ch_layout,out_sample,out_sample_rate
            ,in_ch_layout,in_sample,in_sample_rate,0,NULL);
    swr_init(swrContext);
    LOGE("swrContext init");
    int out_channerl_nb = av_get_channel_layout_nb_channels(out_ch_layout);

    //    反射得到Class类型
    jclass audioPlayercls = env->GetObjectClass(audioPlayer);
//    反射得到createAudio方法
    jmethodID createAudio = env->GetMethodID(audioPlayercls, "createAudio", "(II)V");
//    反射调用createAudio
    env->CallVoidMethod(audioPlayer, createAudio, 44100, out_channerl_nb);
    LOGE("createAudio");
    jmethodID audio_write = env->GetMethodID(audioPlayercls, "playTrack", "([BI)V");

    while (av_read_frame(pAVFormatContext, avPacket)>=0) {
        int got_frame_ptr = 0;
        avcodec_decode_audio4(avCodecContext, srcAVFram, &got_frame_ptr, avPacket);
        if (got_frame_ptr > 0) {
            LOGE("解码");
            //
            //int swr_convert(struct SwrContext *s, uint8_t **out, int out_count,const uint8_t **in , int in_count);
            swr_convert(swrContext, &out_buffer, 2 * 44100, (const uint8_t **) srcAVFram->data,
                        srcAVFram->nb_samples);

            int size = av_samples_get_buffer_size(NULL,out_channerl_nb, srcAVFram->nb_samples, out_sample, 1);

            jbyteArray audio_sample_array = env->NewByteArray(size);
            env->SetByteArrayRegion(audio_sample_array, 0, size, (const jbyte *) out_buffer);
            env->CallVoidMethod(audioPlayer, audio_write, audio_sample_array, size);
            env->DeleteLocalRef(audio_sample_array);
        }
    }


    avformat_free_context(pAVFormatContext);
    env->ReleaseStringUTFChars(inputStr_, inputStr);
}


//  java 方法 和c 方法 关联
static const JNINativeMethod methods[] = {
        {
                "mp4ToYuv",  "(Ljava/lang/String;Ljava/lang/String;)V",     (void *) mp4_to_yuv
        },
        {
                "player",    "(Ljava/lang/String;Landroid/view/Surface;)V", (void *) player
        },
        {
                "mp3Player", "(Ljava/lang/String;Lcom/hubing/ffmpeg/AudioPlayer;)V",     (void *) MP3_to_pcm
        }
};

/**
 *  注册java 方法
 * @param engv
 * @return
 */
static int registerNatives(JNIEnv *engv) {
    LOGI("registerNatives begin");
    jclass clazz;
    clazz = engv->FindClass("com/hubing/ffmpeg/FFmpegUtils");

    if (clazz == NULL) {
        LOGI("clazz is null");
        return JNI_FALSE;
    }

    if (engv->RegisterNatives(clazz, methods, NELEM(methods)) < 0) {
        LOGI("RegisterNatives error");
        return JNI_FALSE;
    }

    return JNI_TRUE;
}


/**
 * 加载jvm  注册java方法
 */
JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {

    LOGI("jni_OnLoad begin");

    JNIEnv *env = NULL;
    jint result = -1;

    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGI("ERROR: GetEnv failed\n");
        return -1;
    }
    assert(env != NULL);

    registerNatives(env);

    return JNI_VERSION_1_4;
}
}
