//
// Created by 谢玉海 on 2018/6/9.
//

#ifndef FFMPEGSYNC1_FFMPEGAUDIO_H
#define FFMPEGSYNC1_FFMPEGAUDIO_H

#endif //FFMPEGSYNC1_FFMPEGAUDIO_H

#include <pthread.h>
#include <queue>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

extern "C" {
#include <libswresample/swresample.h>
#include <libavcodec/avcodec.h>
#include "Log.h"


//成员变量
class FFmpegAudio {

public:

    FFmpegAudio();

    ~FFmpegAudio();

    int get(AVPacket *avPacket);

    int put(AVPacket *packet);

    void play();

    void stop();

    int createFFmpeg(FFmpegAudio *audio);

    int createPlayer();

    void setAvCodeContext(AVCodecContext *avCodecContext);


public :
    //是否正在播放
    int isPlay;
    //流索引
    int index;

    //音频队列
    std::queue<AVPacket *> myqueue;
    //处理线程
    pthread_t p_playid;

    AVCodecContext *avCodecContext;

    // 互斥锁
    pthread_mutex_t mutex;
    //条件变量
    pthread_cond_t cond;


    ///-----
    SwrContext *swrContext;
    uint8_t *out_buffer;


    //double从第一帧开始算
    double clock;

    //
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine = NULL;
    SLObjectItf outputMixObject = NULL;

//SLEnvironmentalReverbItf
    SLEnvironmentalReverbItf reverbItf = NULL;
    SLresult sLresult;
    SLEnvironmentalReverbSettings settings = SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT;

//队列缓冲区
    SLAndroidSimpleBufferQueueItf bqPlayerQueue = NULL;
//播放器
    SLObjectItf bgPlayerObject = NULL;
    SLPlayItf bqPlayerPlay = NULL;
//音量对象
    SLVolumeItf bqPlayerVolume = NULL;
    size_t bufferSize = 0;
    void *buffer;

};

};