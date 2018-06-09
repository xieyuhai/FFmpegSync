
#include <jni.h>
#include <string>

extern "C" {
//编码
#include <libavcodec/avcodec.h>
//封装格式处理
#include <libavformat/avformat.h>
//
#include <libswresample/swresample.h>
//像素处理
#include <libswscale/swscale.h>

#include <android/native_window_jni.h>
#include <unistd.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>


}

#include "FFmpegAudio.h"
#include "FFmpegVideo.h"
#include "Log.h"


const char *path;
FFmpegAudio *audio;
FFmpegVideo *video;
pthread_t pid;

int isPlay = 0;//播放状态

//解码函数
void *process(void *args) {
    // TODO

    //注册组件
    av_register_all();
    //网络初始化
    avformat_network_init();
//分装格式上下文
    AVFormatContext *pContext = avformat_alloc_context();
//打开视频文件
    if (avformat_open_input(&pContext, path, NULL, NULL)) {
        LOGE("打开失败");
    }
//获取视频信息
    if (avformat_find_stream_info(pContext, NULL) < 0) {
        LOGE("获取信息失败");
    }

    //找到音频流
    for (int i = 0; i < pContext->nb_streams; ++i) {

        //mp3的解码器 获取到解码器上下文
        AVCodecContext *pCodecContext = pContext->streams[i]->codec;
        //获取解码器
        AVCodec *pCodec = avcodec_find_decoder(pCodecContext->codec_id);


        if (avcodec_open2(pCodecContext, pCodec, NULL) < 0) {
            LOGE("解码器无法打开");
            continue;
        }

        LOGE("循环  %d", i);
        //codec 每一个流对应的解码上下文，codec_type流的类型
        if (pContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
//            audio_stream_index = i;
            video->setAvCodeContext(pCodecContext);
            video->index = i;
        } else if (pContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio->setAvCodeContext(pCodecContext);
            audio->index = i;
        }
//        sleep(1);
    }




    //开启音视频播放死循环
//    video->play();
    audio->play();
    isPlay = 1;


    //解码packet
    AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));

    //解码完整个视频  子线程

    while (isPlay && av_read_frame(pContext, packet) == 0) {
        //如果这个packet   流索引等于  视频流索引  添加到视频队列

        if (video && video->isPlay && packet->stream_index == video->index) {
//            video->put(packet);
        } else if (audio && audio->isPlay && packet->stream_index == audio->index) {
            audio->put(packet);
        }
        usleep(26 * 1000);

        av_packet_unref(packet);
    }

    //视频解码完
    isPlay = 0;
    if (video && video->isPlay) {
        video->stop();
    }

    if (audio && audio->isPlay) {
        audio->stop();
    }

    av_free_packet(packet);
    avformat_free_context(pContext);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_ocean_ffmpeg_MusicPlayer_player(JNIEnv *env, jobject instance) {


}


extern "C"
JNIEXPORT void JNICALL
Java_com_ocean_ffmpeg_MusicPlayer_play(JNIEnv *env, jobject instance, jstring path_) {
    path = env->GetStringUTFChars(path_, 0);

    //实例化
    video = new FFmpegVideo;
    audio = new FFmpegAudio;

    pthread_create(&pid, NULL, process, NULL);

    //env->ReleaseStringUTFChars(path_, path);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ocean_ffmpeg_MusicPlayer_display(JNIEnv *env, jobject instance, jobject surface) {


}

extern "C"
JNIEXPORT void JNICALL
Java_com_ocean_ffmpeg_MusicPlayer_stop(JNIEnv *env, jobject instance) {


}

extern "C"
JNIEXPORT void JNICALL
Java_com_ocean_ffmpeg_MusicPlayer_realese(JNIEnv *env, jobject instance) {


}