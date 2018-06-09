//
// Created by 谢玉海 on 2018/6/9.
//

#ifndef FFMPEGSYNC1_FFMPEGVIDEO_H
#define FFMPEGSYNC1_FFMPEGVIDEO_H

#endif //FFMPEGSYNC1_FFMPEGVIDEO_H

#include <pthread.h>
#include <queue>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include "Log.h"


//成员变量
class FFmpegVideo {

public:
    FFmpegVideo();

    ~FFmpegVideo();

    int get(AVPacket *avPacket);

    int put(AVPacket *packet);

    void play();

    void stop();


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
};

};