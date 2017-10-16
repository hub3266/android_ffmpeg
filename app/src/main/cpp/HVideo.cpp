//
// Created by lenovo on 2017/10/9.
//

#include <pthread.h>
#include "HVideo.h"
#include "Log.h"

static void (*video_call)(AVFrame *frame);

void *play_vedio(void *arg) {
    HVideo *video = (HVideo *) arg;
    //像素数据（解码数据）
    AVFrame *frame = av_frame_alloc();
    AVFrame *rgb_frame = av_frame_alloc();
    //转换rgba
//    SwsContext *sws_ctx = sws_getContext(
//            video->codec->width, video->codec->height,video->codec->pix_fmt,
//            video->codec->width,video->codec->height, AV_PIX_FMT_RGBA,
//            SWS_BILINEAR, 0, 0, 0);
    //    给缓冲区分配内存
    //只有指定了AVFrame的像素格式、画面大小才能真正分配内存
    //缓冲区分配内存
    LOGE("HVideo::play_vedio 宽-  %d,  高-  %d  ", video->codec->width, video->codec->height);
    uint8_t *out_buffer = (uint8_t *) av_mallocz(
            avpicture_get_size(AV_PIX_FMT_RGBA, video->codec->width, video->codec->height));
//设置yuvFrame的缓冲区，像素格式
    int re = avpicture_fill((AVPicture *) rgb_frame, out_buffer, AV_PIX_FMT_RGBA,
                            video->codec->width, video->codec->height);
    LOGE("HVideo::play_vedio 申请内存%d   ", re);
    SwsContext *swsContext = sws_getContext(video->codec->width, video->codec->height,
                                            video->codec->pix_fmt,
                                            video->codec->width, video->codec->height,
                                            AV_PIX_FMT_RGBA,
                                            SWS_BICUBIC, NULL, NULL, NULL);
    //编码数据
    LOGE("HVideo::play_vedio 初始化 packet");
    AVPacket *packet = (AVPacket *) av_mallocz(sizeof(AVPacket));
    int got_picture_ptr;
    LOGE("HVideo::play_vedio  进入循环 %d", video->isPlay)
    while (video->isPlay) {
        video->get(packet);
        LOGE("HVideo::play_vedio  获取 packet");
        avcodec_decode_video2(video->codec, frame, &got_picture_ptr, packet);
        if (!got_picture_ptr) {
            continue;
        }
        //        转码成rgb
        int code = sws_scale(swsContext, (const uint8_t *const *) frame->data, frame->linesize,
                             0, video->codec->height,
                             rgb_frame->data, rgb_frame->linesize);
        video_call(rgb_frame);
    }

    LOGE("HVideo::play_vedio free packet");
    av_free(packet);
    LOGE("HVideo::play_vedio free packet ok");
    LOGE("HVideo::play_vedio free packet");
    av_frame_free(&frame);
    av_frame_free(&rgb_frame);
    sws_freeContext(swsContext);
    size_t size = video->queue.size();
    for (int i = 0; i < size; ++i) {
        AVPacket *pkt = video->queue.front();
        av_free(pkt);
        video->queue.pop();
    }
    LOGE("VIDEO EXIT");
    pthread_exit(0);
}

HVideo::HVideo() {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
}

HVideo::~HVideo() {
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
}

void HVideo::setAvCodecContext(AVCodecContext *codecContext) {
    this->codec = codecContext;

}

void HVideo::setHAudio(HAudio *hAudio) {
    this->hAudio = hAudio;
}

void HVideo::setPlayCall(void (*call1)(AVFrame *)) {
    video_call = call1;
}

int HVideo::put(AVPacket *packet) {
    AVPacket *packet1 = (AVPacket *) av_mallocz(sizeof(AVPacket));
    //if (av_packet_copy_props(packet1, packet) < 0) {}
    if (av_copy_packet(packet1, packet)) {
        LOGI("拷贝失败！！");
        return 0;
    }
    LOGI("加锁");
    pthread_mutex_lock(&mutex);
    LOGI("packet1 加入队列");
    queue.push(packet1);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    LOGI("解锁");
    return 1;
}

int HVideo::get(AVPacket *packet) {
    LOGI("[HVideo::get]  进入get");
    pthread_mutex_lock(&mutex);
    while (isPlay) {
        if (!queue.empty()) {
            LOGI("[HVideo::get]  拷贝packet");
            int ret_1 = av_packet_ref(packet, queue.front());
            if (ret_1 != 0) {
                LOGI("[HVideo::get]  拷贝失败 ");
                break;
            }
            //取成功了  弹出队列  销毁packet
            LOGI("[HVideo::get] 取成功了  弹出队列  销毁packet");
            AVPacket *pkt = queue.front();
            queue.pop();
            av_free(pkt);
            break;
        } else {
            //没有找到packet数据 一直等待
            LOGI("[HVideo::get] 队列为空 等待队列数据");
            pthread_cond_wait(&cond, &mutex);
        }
    }
    LOGI("[HVideo::get] get  结束循环");
     pthread_mutex_unlock(&mutex);
    LOGI("fin get ");
    return 0;

}

void HVideo::play() {
    isPlay = 1;
    pthread_create(&p_playid, 0, play_vedio, this);
}

void HVideo::stop() {

}
