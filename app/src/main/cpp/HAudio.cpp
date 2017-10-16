//
// Created by lenovo on 2017/10/9.
//
#include "HAudio.h"
int createFFmpeg(HAudio *);
HAudio::HAudio() {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

}


void HAudio::setAvCodecContext(AVCodecContext *codecContext) {
    this->codecContext = codecContext;
    createFFmpeg(this);

}

int HAudio::put(AVPacket *packet) {
    LOGI(" HAudio::put");
    AVPacket *packet1 = (AVPacket *) av_mallocz(sizeof(AVPacket));
    //if (av_packet_copy_props(packet1, packet) < 0) {}
//    if (av_copy_packet(packet1, packet)) {
//        LOGI(" HAudio::put拷贝失败！！");
//        return 0;
//    }
    if (av_packet_ref(packet1, packet)) {
        LOGE("HAudio av_packet_ref 错误 ");
//        克隆失败
        return 0;
    }
    pthread_mutex_lock(&mutex);
    queue.push(packet1);
    LOGE("HAudio 压入一帧音频数据  队列%d ",queue.size());
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    return 0;
}

int HAudio::get(AVPacket *packet) {
    pthread_mutex_lock(&mutex);
    while (isPlay) {
        if (!queue.empty()) {
            //int ret_1 = av_packet_ref(packet, queue.front());
            if (av_packet_ref(packet, queue.front())) {
                break;
            }
            //取成功了  弹出队列  销毁packet
            AVPacket *pkt = queue.front();
            queue.pop();
            LOGE("HAudio 取出一 个音频帧%d",queue.size());
            av_free(pkt);
            break;
        } else {
            //没有找到packet数据 一直等待
            pthread_cond_wait(&cond, &mutex);
        }
    }
    pthread_mutex_unlock(&mutex);
    return 0;
}

void *play_audio(void *args) {
    HAudio *audio = (HAudio *) args;
    int ret = audio->createAudioPlayer();
    LOGI("HAudio,  audio->createAudioPlayer() %d",ret);
    pthread_exit(0);

}

/**
 * 获取swrContext
 * @param audio
 * @return
 */
int createFFmpeg(HAudio* audio){
    LOGE("HAudio createFFmpeg")
    audio->swrContext = swr_alloc();
    audio->out_buffer = (uint8_t *) av_mallocz(44100 * 2);
    uint64_t  out_ch_layout=AV_CH_LAYOUT_STEREO;
    //    输出采样位数  16位
    enum AVSampleFormat out_formart=AV_SAMPLE_FMT_S16;
    //输出的采样率必须与输入相同
    int out_sample_rate = audio->codecContext->sample_rate;
    swr_alloc_set_opts(audio->swrContext,out_ch_layout,out_formart,out_sample_rate,
                       audio->codecContext->channel_layout, audio->codecContext->sample_fmt,  audio->codecContext->sample_rate,
                       0,NULL);

    swr_init(audio->swrContext);
    //    获取通道数  2
    audio->out_channer_nb = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    LOGE("------>通道数%d  ", audio->out_channer_nb);
    return 1;
}

int getPcm(HAudio* audio){
    LOGI("HAudio getPcm START ");
    AVPacket* packet = (AVPacket *) av_mallocz(sizeof(AVPacket));
    AVFrame* frame = av_frame_alloc();
    int got_frame_ptr;
    int bufferSize;
    LOGI("HAudio getPcm 进入循环");
    while(audio->isPlay){
        LOGI("HAudio getPcm 获取 packet");
        audio->get(packet);
        avcodec_decode_audio4(audio->codecContext,frame,&got_frame_ptr,packet);
        LOGI("HAudio getPcm got_frame_ptr %d",got_frame_ptr);
        if(got_frame_ptr) {
            swr_convert(audio->swrContext, &audio->out_buffer, 44100 * 2,
                        (const uint8_t **) frame->data, frame->nb_samples);
            bufferSize = av_samples_get_buffer_size(NULL, audio->out_channer_nb, frame->nb_samples,
                                                     AV_SAMPLE_FMT_S16, 1);
            break;
        }
    }
    av_free(packet);
    av_frame_free(&frame);
    LOGI("HAudio getPcm STOP ");
    return bufferSize;

}

void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq,void *pContext){
    LOGI("HAudio bqPlayerCallback");
    HAudio* audio =(HAudio*) pContext;
    SLresult result;
    //void* buffer;
    int bufferSize;
    bufferSize = getPcm(audio);
    if(bufferSize>0) {
        result = (*bq)->Enqueue(bq, audio->out_buffer, bufferSize);
       //assert(SL_RESULT_SUCCESS == result);
    }else{
        LOGI("解码错误");
    }
    LOGI("HAudio bqPlayerCallback  fin");

}
int HAudio::createAudioPlayer() {
    SLresult result;
    // 创建引擎engineObject
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    if (SL_RESULT_SUCCESS != result) {
        return 0;
    }
    // 实现引擎engineObject
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return 0;
    }
    // 获取引擎接口engineEngine
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE,
                                           &engineEngine);
    if (SL_RESULT_SUCCESS != result) {
        return 0;
    }
    // 创建混音器outputMixObject
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0,
                                              0, 0);
    if (SL_RESULT_SUCCESS != result) {
        return 0;
    }
    // 实现混音器outputMixObject
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return 0;
    }
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentalReverb);
    const SLEnvironmentalReverbSettings settings = SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT;
    if (SL_RESULT_SUCCESS == result) {
        (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &settings);
    }


    //======================
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            2};
    SLDataFormat_PCM pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1, SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
                            SL_BYTEORDER_LITTLEENDIAN};
//   新建一个数据源 将上述配置信息放到这个数据源中
    SLDataSource slDataSource = {&android_queue, &pcm};
//    设置混音器
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};

    SLDataSink audioSnk = {&outputMix, NULL};
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND,
            /*SL_IID_MUTESOLO,*/ SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
            /*SL_BOOLEAN_TRUE,*/ SL_BOOLEAN_TRUE};
    //先讲这个
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &slDataSource,
                                                &audioSnk, 2,
                                                ids, req);
    LOGE("HAudio 创建播放器 result %d", result);


    //初始化播放器
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    LOGE("HAudio  创建播放器 Realize result %d", result);

//    得到接口后调用  获取Player接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    LOGE("HAudio result %d", result);

//    注册回调缓冲区 //获取缓冲队列接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
                                    &bqPlayerBufferQueue);
    //缓冲接口回调
    (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, this);
//    获取音量接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME, &bqPlayerVolume);

//    获取播放状态接口
    (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);

    bqPlayerCallback(bqPlayerBufferQueue, this);
    return 1;
}

void HAudio::play() {
    LOGI("HAudio::play()");
    isPlay = 1;
    pthread_create(&p_playid, NULL, play_audio, this);
}


void HAudio::stop() {
    LOGE("声音暂停");
    //因为可能卡在 deQueue
    pthread_mutex_lock(&mutex);
    isPlay = 0;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    pthread_join(p_playid, 0);
    if (bqPlayerPlay) {
        (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_STOPPED);
        bqPlayerPlay = 0;
    }
    if (bqPlayerObject) {
        (*bqPlayerObject)->Destroy(bqPlayerObject);
        bqPlayerObject = 0;

        bqPlayerBufferQueue = 0;
        bqPlayerVolume = 0;
    }

    if (outputMixObject) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = 0;
    }

    if (engineObject) {
        (*engineObject)->Destroy(engineObject);
        engineObject = 0;
        engineEngine = 0;
    }
    if (swrContext)
        swr_free(&swrContext);
    if (this->codecContext) {
        if (avcodec_is_open(this->codecContext))
            avcodec_close(this->codecContext);
        avcodec_free_context(&this->codecContext);
        this->codecContext = 0;
    }
    LOGE("AUDIO clear");
}

HAudio::~HAudio() {
    if (out_buffer) {
        free(out_buffer);
    }
    for (int i = 0; i < queue.size(); ++i) {
        AVPacket *pkt = queue.front();
        queue.pop();
        LOGE("销毁音频帧%d",queue.size());
        av_free(pkt);
    }
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
}
