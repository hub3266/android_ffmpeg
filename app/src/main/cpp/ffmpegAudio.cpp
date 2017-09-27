//
// Created by lenovo on 2017/9/26.
//

#include "ffmpegAudio.h"

extern "C" {
AVFormatContext *pContext;
AVCodecContext *pCodecContext;
AVCodec *pCodec;
AVPacket *avPacket;
AVFrame *frame;
SwrContext * swrContext;
int out_channer_nb;
uint8_t *out_buffer;
int createffmpeg(const char *input,int *rate,int *channel) {
    av_register_all();
    pContext = avformat_alloc_context();
    avformat_open_input(&pContext, input, NULL, NULL);
    avformat_find_stream_info(pContext, NULL);

    int audio_stream_idx = -1;
    for (int i = 0; i < pContext->nb_streams; i++) {
        if (pContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream_idx = i;
        }
    }

    pCodecContext = pContext->streams[audio_stream_idx]->codec;
    pCodec = avcodec_find_decoder(pCodecContext->codec_id);
    if (avcodec_open2(pCodecContext, pCodec, NULL) < 0) {
        LOGE("获取编码失败");
    }

    avPacket = (AVPacket *) av_malloc(sizeof(AVPacket));
    frame = av_frame_alloc();
    int64_t out_ch_layout = AV_CH_LAYOUT_STEREO;
    //输出的采样率必须与输入相同
    int out_sample_rate = pCodecContext->sample_rate;
     swrContext = swr_alloc();
    swr_alloc_set_opts(swrContext,out_ch_layout,AV_SAMPLE_FMT_S16,out_sample_rate,
    pCodecContext->channel_layout,pCodecContext->sample_fmt,pCodecContext->sample_rate,0,0);
    swr_init(swrContext);
//    获取通道数  2
    out_channer_nb = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    //为什么是输入的采样率 和通道数
    *rate = pCodecContext->sample_rate;
    *channel = pCodecContext->channels;
    out_buffer = (uint8_t *) av_malloc(44100 * 2);
    LOGE("初始化FFmpeg完毕");
    return 0;
}
int getPCM(void **pcm,size_t *pcm_size) {

    int got_frame;
    while(av_read_frame(pContext,avPacket)>=0){
        avcodec_decode_audio4(pCodecContext,frame,&got_frame,avPacket);
        if(got_frame){
            swr_convert(swrContext, &out_buffer,44100*2, (const uint8_t **) frame->data, frame->sample_rate);
            size_t size =av_samples_get_buffer_size(NULL,out_channer_nb,frame->nb_samples,AV_SAMPLE_FMT_S16,frame->nb_samples);
            *pcm = out_buffer;
            *pcm_size = size;
            LOGI("解码");
            break;
        }
    }

    return 0;
}
 void release() {
    av_free_packet(avPacket);
    av_free(out_buffer);
    av_frame_free(&frame);
    swr_free(&swrContext);
    avcodec_close(pCodecContext);
    avformat_close_input(&pContext);
}

}
