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
    //av_image_fill_arrays()
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


    double  last_play  //上一帧的播放时间
    ,play             //当前帧的播放时间
    , last_delay    // 上一次播放视频的两帧视频间隔时间
    ,delay         //两帧视频间隔时间
    ,audio_clock //音频轨道 实际播放时间
    ,diff   //音频帧与视频帧相差时间
    ,sync_threshold
    ,start_time  //从第一帧开始的绝对时间
    ,pts
    ,actual_delay//真正需要延迟时间
    ;//两帧间隔合理间隔时间
    start_time = av_gettime() / 1000000.0;
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
        if ((pts = av_frame_get_best_effort_timestamp(frame)) == AV_NOPTS_VALUE) {
            pts = 0;
        }
        play = pts * av_q2d(video->time_base);
//        纠正时间
        play = video->synchronize(frame, play);
        delay = play - last_play;
        if (delay <= 0 || delay > 1) {
            delay = last_delay;
        }
        audio_clock = video->hAudio->clock;
        last_delay = delay;
        last_play = play;
//音频与视频的时间差
        diff = video->clock - audio_clock;
//        在合理范围外  才会延迟  加快

        sync_threshold = (delay > 0.01 ? 0.01 : delay);

        if (fabs(diff) < 10) {
            if (diff <= -sync_threshold) {
                delay = 0;
            } else if (diff >=sync_threshold) {
                delay = 2 * delay;
            }
        }
        start_time += delay;
        actual_delay=start_time-av_gettime()/1000000.0;
        if (actual_delay < 0.01) {
            actual_delay = 0.01;
        }
        av_usleep(actual_delay*1000000.0+6000);
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

double HVideo::synchronize(AVFrame *frame, double play) {
    //clock是当前播放的时间位置
    if (play != 0)
        clock=play;
    else //pst为0 则先把pts设为上一帧时间
        play = clock;
    //可能有pts为0 则主动增加clock
    //frame->repeat_pict = 当解码时，这张图片需要要延迟多少
    //需要求出扩展延时：
    //extra_delay = repeat_pict / (2*fps) 显示这样图片需要延迟这么久来显示
    double repeat_pict = frame->repeat_pict;
    //使用AvCodecContext的而不是stream的
    double frame_delay = av_q2d(codec->time_base);
    //如果time_base是1,25 把1s分成25份，则fps为25
    //fps = 1/(1/25)
    double fps = 1 / frame_delay;
    //pts 加上 这个延迟 是显示时间
    double extra_delay = repeat_pict / (2 * fps);
    double delay = extra_delay + frame_delay;
//    LOGI("extra_delay:%f",extra_delay);
    clock += delay;
    return play;
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
