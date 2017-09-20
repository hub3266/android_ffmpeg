#include <jni.h>
#include <string>
#include <android/log.h>
extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <assert.h>
}
#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"ffmpeg",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"ffmpeg",FORMAT,##__VA_ARGS__);
# define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))

extern "C" {
JNIEXPORT jstring JNICALL
Java_com_hubing_ffmpeg_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "111111111111111111111111111111";
    av_register_all();
    return env->NewStringUTF(hello.c_str());
}

JNIEXPORT void JNICALL mp4_to_yuv(JNIEnv *env,jobject obj,jstring inputStr_,jstring outputStr_){
    const  char* inputStr = env->GetStringUTFChars(inputStr_,NULL);
    const  char* outputStr = env->GetStringUTFChars(outputStr_,NULL);
    LOGE("inputStr####%s",inputStr);
    LOGE("outputStr####%s",outputStr);
    //注册各大组件
    av_register_all();
    //获取转码上下文
    AVFormatContext * pAVFormatContext = avformat_alloc_context();
    if(avformat_open_input(&pAVFormatContext, inputStr, NULL, NULL)<0 ){
        LOGE("打开文件失败  " );
        return;
    }
    if (avformat_find_stream_info(pAVFormatContext, NULL) < 0) {
        LOGE("获取信息失败");
        return;
    }
    int vedio_stream_idx=-1;
    //找到视频流
    for(int i= 0;i<pAVFormatContext->nb_streams;++i){
        if(pAVFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
            vedio_stream_idx = i;
        }
    }
    LOGE("vedio_stream_idx :%d ",vedio_stream_idx );
    //解码器上下文
    AVCodecContext * pAVCodecContext = pAVFormatContext->streams[vedio_stream_idx]->codec;
    //解码器
    AVCodec *avCodec = avcodec_find_decoder(pAVCodecContext->codec_id);

    if(avcodec_open2(pAVCodecContext,avCodec,NULL)<0){
        LOGE("解码失败");
        return;
    }

    //分配内存
    AVPacket* pAVPacket = (AVPacket *) av_malloc(sizeof(AVPacket));
    //初始化 数据
    av_init_packet(pAVPacket);

    AVFrame * pAVFrame = av_frame_alloc();
    AVFrame * pYuvAVFrame = av_frame_alloc();

    //    给yuvframe  的缓冲区 初始化
    uint8_t* buffSize = (uint8_t *) av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pAVCodecContext->width, pAVCodecContext->height));
    avpicture_fill((AVPicture*)pYuvAVFrame,buffSize,AV_PIX_FMT_YUV420P,pAVCodecContext->width,pAVCodecContext->height);
    LOGE("宽 %d  高 %d",pAVCodecContext->width,pAVCodecContext->height);

    SwsContext* swsContext = sws_getContext(pAVCodecContext->width,pAVCodecContext->height,pAVCodecContext->pix_fmt,
                                           pAVCodecContext->width,pAVCodecContext->height,AV_PIX_FMT_YUV420P,
                                           SWS_BILINEAR, NULL,NULL,NULL);
    FILE *fp_yuv = fopen(outputStr, "wb");

    //packet入参 出参对象  转换上下文
    int got_frame;
    //读取码流中的音频若干帧或者视频一帧
    while(av_read_frame(pAVFormatContext,pAVPacket)>=0){
        //解封装
        // 根据frame 进行原生绘制    bitmap  window
        //got_frame 0 :没有frame
        avcodec_decode_video2(pAVCodecContext, pAVFrame, &got_frame, pAVPacket);

        if(got_frame>0){
            sws_scale(swsContext,(const uint8_t *const *) pAVFrame->data,pAVFrame->linesize,0,pAVFrame->height,pYuvAVFrame->data,pYuvAVFrame->linesize);
            int y_size = pAVCodecContext->width * pAVCodecContext->height;

            fwrite(pYuvAVFrame->data[0], 1, y_size, fp_yuv);
            fwrite(pYuvAVFrame->data[1], 1, y_size/4, fp_yuv);
            fwrite(pYuvAVFrame->data[2], 1, y_size/4, fp_yuv);
        }

        av_free_packet(pAVPacket);
    }
    fclose(fp_yuv);
    sws_freeContext(swsContext);
    av_frame_free(&pAVFrame);
    av_frame_free(&pYuvAVFrame);
    avcodec_close(pAVCodecContext);
    LOGE("11111111111111111");
    avcodec_free_context(&pAVCodecContext);
    avformat_free_context(pAVFormatContext);
    LOGE("222222");
    env->ReleaseStringUTFChars(inputStr_,inputStr);
    env->ReleaseStringUTFChars(outputStr_,outputStr);
    LOGE("complate");

}


//  java 方法 和c 方法 关联
static const  JNINativeMethod methods[]={
        {
                "mp4ToYuv","(Ljava/lang/String;Ljava/lang/String;)V",(void*)mp4_to_yuv
        }
};

/**
 *  注册java 方法
 * @param engv
 * @return
 */
static int registerNatives(JNIEnv* engv)
{
    LOGI("registerNatives begin");
    jclass  clazz;
    clazz = engv -> FindClass( "com/hubing/ffmpeg/MainActivity");

    if (clazz == NULL) {
        LOGI("clazz is null");
        return JNI_FALSE;
    }

    if (engv ->RegisterNatives(clazz, methods, NELEM(methods)) < 0) {
        LOGI("RegisterNatives error");
        return JNI_FALSE;
    }

    return JNI_TRUE;
}


/**
 * 加载jvm  注册java方法
 */
JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
{

    LOGI("jni_OnLoad begin");

    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGI("ERROR: GetEnv failed\n");
        return -1;
    }
    assert(env != NULL);

    registerNatives(env);

    return JNI_VERSION_1_4;
}
}
