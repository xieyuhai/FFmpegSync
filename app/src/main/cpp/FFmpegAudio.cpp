//
// Created by 谢玉海 on 2018/6/9.
//



#include "FFmpegAudio.h"


//构造初始化锁
FFmpegAudio::FFmpegAudio() {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
}


//opensl  es调用 int *rate, int *channel
int FFmpegAudio::createFFmpeg(FFmpegAudio *audio) {

    //mp3里面所包含的编码格式  转换成pcm
    audio->swrContext = swr_alloc();

    /**
     * struct SwrContext *swr_alloc_set_opts(struct SwrContext *s,
                                      int64_t out_ch_layout, enum AVSampleFormat out_sample_fmt, int out_sample_rate,
                                      int64_t  in_ch_layout, enum AVSampleFormat  in_sample_fmt, int  in_sample_rate,
                                      int log_offset, void *log_ctx);
     */
    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;//立体声
    //输出采样位数 16位
    enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    //输出的采样率必须与输入相同
    int out_sample_rate = audio->avCodecContext->sample_rate;


    swr_alloc_set_opts(audio->swrContext,
                       AV_CH_LAYOUT_STEREO,
                       AV_SAMPLE_FMT_S16,
                       audio->avCodecContext->sample_rate,
                       audio->avCodecContext->channel_layout,
                       audio->avCodecContext->sample_fmt,
                       audio->avCodecContext->sample_rate,
                       0,
                       0
    );

    //音频所占字节数=通道数 * 采用频率（Hz） * 采用位数 （byte）
    swr_init(audio->swrContext);


    audio->out_buffer = (uint8_t *) av_malloc(44100 * 2);

    LOGE("初始化FFmpeg完毕");

    return 0;
}

void FFmpegAudio::setAvCodeContext(AVCodecContext *codec) {
    avCodecContext = codec;
    createFFmpeg(this);
}

//pcm入参出参
int getPcm(FFmpegAudio *audio) {

    int frameCount = 0;
    int got_frame;
    AVFrame *frame = av_frame_alloc();

    AVPacket *packet = (AVPacket *) av_mallocz(sizeof(AVPacket));

    int out_channel_nb = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    int out_buffer_size = 0;

    while (audio->isPlay) {
        out_buffer_size = 0;

        audio->get(packet);

//解码   mp3 编码格式 frame转  pcm
        avcodec_decode_audio4(audio->avCodecContext, frame, &got_frame, packet);

        if (got_frame) {
            LOGE("解码");
//                int swr_convert(struct SwrContext *s, uint8_t **out, int out_count,
//                                const uint8_t **in , int in_count);
            swr_convert(audio->swrContext,
                        &audio->out_buffer,
                        44100 * 2,
                        (const uint8_t **) frame->data,
                        frame->nb_samples);

            //通道数
            out_buffer_size = av_samples_get_buffer_size(NULL,
                                                         out_channel_nb,
                                                         frame->nb_samples,
                                                         AV_SAMPLE_FMT_S16, 1);

            break;
        }
    }
    av_free(packet);
    av_frame_free(&frame);
    return out_buffer_size;
}


//回调函数，添加pcm数据到缓冲区
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {

    FFmpegAudio *audio = (FFmpegAudio *) context;

    //取到音频数据了
    LOGE("bqPlayerCallback getPcm  before");
    int size = getPcm(audio);
    LOGE("bqPlayerCallback getPcm after");

    if (size > 0) {
        //播放的关键地方
        SLresult sLresult1 = (*bq)->Enqueue(bq, audio->out_buffer, size);
        LOGE("正在播放%d ", sLresult1);
    }
}

int FFmpegAudio::createPlayer() {
    //初始化一个引擎
    sLresult = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);

    if (SL_RESULT_SUCCESS != sLresult) {
        return 0;
    }
    sLresult = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);

    if (SL_RESULT_SUCCESS != sLresult) {
        return 0;
    }
    //获取到引擎接口
    sLresult = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    if (SL_RESULT_SUCCESS != sLresult) {
        return 0;
    }
    LOGE("引擎地址%p   SLresult %d  ", engineEngine, sLresult);

    //创建混音器
    sLresult = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, 0, 0);
    if (SL_RESULT_SUCCESS != sLresult) {
        return 0;
    }
    sLresult = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != sLresult) {
        return 0;
    }
    //设置环境混响
    sLresult = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                                &reverbItf);

    LOGE("播放器接口6 %d", sLresult);
    //设置混音器
    if (SL_RESULT_SUCCESS == sLresult) {
        (*reverbItf)->SetEnvironmentalReverbProperties(
                reverbItf, &settings);
    }
    //    初始化FFmpeg
    int rate;
    int channels = 2;
    // createFFmpeg(&rate, &channels);
    LOGE("初始化FFmpeg完毕");
    SLDataLocator_AndroidBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    /**
    typedef struct SLDataFormat_PCM_ {
        SLuint32 		formatType;  pcm
        SLuint32 		numChannels;  通道数
        SLuint32 		samplesPerSec;  采样率
        SLuint32 		bitsPerSample;  采样位数
        SLuint32 		containerSize;  包含位数
        SLuint32 		channelMask;     立体声
        SLuint32		endianness;    end标志位
    } SLDataFormat_PCM;
     */
    SLuint32 ch = NULL;
    if (channels == 2) {
        ch = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    } else if (channels == 1) {
        ch = SL_SPEAKER_FRONT_LEFT;

    }

//    SLAndroidDataFormat_PCM_EX
    SLDataFormat_PCM pcm = {SL_DATAFORMAT_PCM, 2/*channels*/, /*rate * 1000*/ SL_SAMPLINGRATE_44_1,
                            SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                            ch,
                            SL_BYTEORDER_LITTLEENDIAN};
    SLDataSource slDataSource = {&android_queue, &pcm};
//    设置混音器
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&outputMix, NULL};
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    LOGE("引擎  %p", engineEngine);
//    混音器关联起来
//创建一个播放器
    sLresult = (*engineEngine)->CreateAudioPlayer(engineEngine, &bgPlayerObject, &slDataSource,
                                                  &audioSnk, 3,
                                                  ids, req);
    LOGE("   sLresult  %d   bgPlayerObject  %p  slDataSource  %p", sLresult, bgPlayerObject,
         slDataSource);
    //初始化播放器
    sLresult = (*bgPlayerObject)->Realize(bgPlayerObject, SL_BOOLEAN_FALSE);
    LOGE(" sLresult  2 %d", sLresult);
//创建一个播放器接口
    sLresult = (*bgPlayerObject)->GetInterface(bgPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    LOGE(" sLresult 3  %d", sLresult);
//    注册缓冲区
    sLresult = (*bgPlayerObject)->GetInterface(bgPlayerObject, SL_IID_BUFFERQUEUE, &bqPlayerQueue);
    LOGE(" sLresult 4  %d", sLresult);
//    设置回调接口   getPCm  pthread 函数

    //缓冲接口回调
    sLresult = (*bqPlayerQueue)->RegisterCallback(bqPlayerQueue, bqPlayerCallback, this);
    LOGE(" sLresult  5  %d", sLresult);
// 获取音量接口
    (*bgPlayerObject)->GetInterface(bgPlayerObject, SL_IID_VOLUME, &bqPlayerVolume);
    //获取播放状态接口
    (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);

//    播放第一帧
    bqPlayerCallback(bqPlayerQueue, this);
    return 1;
}


int FFmpegAudio::get(AVPacket *packet) {
    pthread_mutex_lock(&mutex);

    while (isPlay) {
        if (!myqueue.empty()) {
            //从队列去除一个packet  clone一个给入参对象
            if (av_packet_ref(packet, myqueue.front()) < 0) {
                break;
            }

            //取出成功，弹出队列  销毁packet
            AVPacket *pkt = myqueue.front();
            myqueue.pop();

//            av_free_packet(pkt);
            av_free(pkt);

            break;

        } else {
            //如果队列中没有数据，一直等待，阻塞式
            pthread_cond_wait(&cond, &mutex);
        }
    }
    pthread_mutex_unlock(&mutex);
    return 0;
}

int FFmpegAudio::put(AVPacket *packet) {
    AVPacket *pkt = (AVPacket *) av_malloc(sizeof(AVPacket));

    if (av_packet_ref(pkt, packet)) {
        LOGE("clone 失败");
        return 0;
    }
    LOGE("压入一帧音频数据-------------------------");

    pthread_mutex_lock(&mutex);
    myqueue.push(pkt);
    //通知消费者，解锁
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    return 1;
}

void *play_audio(void *arg) {
//    LOGE("开启音频线程");
//    FFmpegAudio *audio = (FFmpegAudio *) arg;
//
//    AVFrame *frame = av_frame_alloc();
//    AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));
//
//    //
//    SwrContext *swrContext = swr_alloc();
//
//
//    swr_alloc_set_opts(swrContext,
//                       AV_CH_LAYOUT_STEREO,
//                       AV_SAMPLE_FMT_S16,
//                       audio->avCodecContext->sample_rate,//输出的采样率必须与输入相同
//                       audio->avCodecContext->channel_layout,
//                       audio->avCodecContext->sample_fmt,
//                       audio->avCodecContext->sample_rate,
//                       0,
//                       NULL
//    );
//
//
//    swr_init(swrContext);
//
//
//    uint8_t *out_buffer = (uint8_t *) av_malloc(44100 * 2 * 2);
//    //获取通道数
//    int channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
//
//    int got_frame;
//    //死循环
//    while (audio->isPlay) {
//        LOGE("消费音频一帧数据 ------解码");
//
//        audio->get(packet);
//        avcodec_decode_audio4(audio->avCodecContext, frame, &got_frame, packet);
//
//        if (got_frame) {
//            LOGE("解码音视频");
//            swr_convert(swrContext, &out_buffer, 44100 * 2 * 2, (const uint8_t **) frame->data,
//                        frame->nb_samples);
//
//            int out_buffer_size = av_samples_get_buffer_size(NULL, channels, frame->nb_samples,
//                                                             AV_SAMPLE_FMT_S16, 1);
//
//            //执行播放  得到了pcm 的out_buffer 数据
//        }
//    }
//
//    LOGE("初始化FFmpeg完毕");


    FFmpegAudio *audio = (FFmpegAudio *) arg;
    //不断处于播放状态  阻塞
    audio->createPlayer();
    //播放
    pthread_exit(0);

//    return 0;

}


void FFmpegAudio::play() {
    isPlay = 1;
    int ret = pthread_create(&p_playid, NULL, play_audio, this);
    if (ret < 0) {
        LOGE("ERROR ret");
    }
}

void FFmpegAudio::stop() {

}




