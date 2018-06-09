//
// Created by 谢玉海 on 2018/6/9.
//


#include "FFmpegVideo.h"


FFmpegVideo::FFmpegVideo() {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
}


int FFmpegVideo::get(AVPacket *packet) {
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

int FFmpegVideo::put(AVPacket *packet) {
    AVPacket *pkt = (AVPacket *) av_malloc(sizeof(AVPacket));

    if (av_packet_ref(pkt, packet)) {
        LOGE("clone 失败");
        return 0;
    }
    LOGE("压入一帧视频数据----------------------");

    pthread_mutex_lock(&mutex);
    myqueue.push(pkt);
    //通知消费者，解锁
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    return 0;
}


void *play_video(void *args) {
    FFmpegVideo *video = (FFmpegVideo *) args;
    //像素数据（解码数据）
    AVFrame *frame = av_frame_alloc();
    AVFrame *rgb_frame = av_frame_alloc();

    //只有指定了AVFrame 的像素格式，画面大小才能真正分配内存
    //缓冲区分配内存


    uint8_t *out_buffer = (uint8_t *) av_malloc(avpicture_get_size(AV_PIX_FMT_RGBA,
                                                                   video->avCodecContext->width,
                                                                   video->avCodecContext->height
    ));
    //设置YUVFrame的缓冲区，像素格式
    avpicture_fill((AVPicture *) rgb_frame, out_buffer, AV_PIX_FMT_RGBA,
                   video->avCodecContext->width,
                   video->avCodecContext->height);
    //native绘制

    //由于解码出来的帧格式不是RGBA的，在渲染之前需要进行格式转换
    SwsContext *swsContext = sws_getContext(
            video->avCodecContext->width,
            video->avCodecContext->height,
            video->avCodecContext->pix_fmt,
            video->avCodecContext->width,
            video->avCodecContext->height,
            AV_PIX_FMT_RGBA,
            SWS_BICUBIC,
            NULL, NULL, NULL);

    int len, got_frame, frameCount = 0;

    AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));

    while (video->isPlay) {
        LOGE("消费视频一帧数据 ------解码");
        //消费者取到一帧数据  没有就阻塞
        video->get(packet);
        len = avcodec_decode_video2(video->avCodecContext, frame, &got_frame, packet);
        //转换成rgba
        sws_scale(swsContext, (const uint8_t *const *) frame->data, frame->linesize, 0,
                  video->avCodecContext->height,
                  rgb_frame->data, rgb_frame->linesize);

        //界面绘制  拿到了rgb和pcm数据
    }

    //播放完成

    return 0;

}


void FFmpegVideo::play() {
    isPlay = 1;
    int ret = pthread_create(&p_playid, NULL, play_video, this);
    if (ret < 0) {
        LOGE("ERROR ret");
    }
}

void FFmpegVideo::stop() {

}

void FFmpegVideo::setAvCodeContext(AVCodecContext *codec) {
    avCodecContext = codec;
}
