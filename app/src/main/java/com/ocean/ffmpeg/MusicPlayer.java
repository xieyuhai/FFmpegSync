package com.ocean.ffmpeg;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.view.Surface;

/**
 * 先切换到class目录，然后执行下面的命令
 * javap -s MusicPlayer
 * <p>
 * Created by xieyuhai on 2018/6/6.
 * <p>
 * ([BI)V
 * <p>
 * javah
 */

public class MusicPlayer {

    static {
        System.loadLibrary("avcodec-56");
//        System.loadLibrary("avdevice-56");
//        System.loadLibrary("avfilter-5");
        System.loadLibrary("avformat-56");
//        System.loadLibrary("avutil-54");
//        System.loadLibrary("postproc-53");
        System.loadLibrary("swresample-1");
        System.loadLibrary("swscale-3");
        System.loadLibrary("native-lib");
    }


    public native void player();

    public native void play(String path);

    public native void display(Surface surface);

    public native void stop();

    public native void realese();


    //    public native void player(String input, String output);
//    public native void player();
//
//    public native void replayer();
//
//    public native void pause();
//
////    public native void stop();
//
//
//    public native void setVolumeAudioPlayer(int volume);
//
//
//    private AudioTrack audioTrack;
//
//    public void createAudio(int sampleRateInHz, int nb_channels) {
//        int channelConfig;
//        if (nb_channels == 1) {
//            channelConfig = AudioFormat.CHANNEL_OUT_MONO;
//        } else if (nb_channels == 2) {
//            channelConfig = AudioFormat.CHANNEL_IN_STEREO;
//        } else {
//            channelConfig = AudioFormat.CHANNEL_OUT_MONO;
//        }
//        int bufferSize = AudioTrack.getMinBufferSize(sampleRateInHz, channelConfig,
//                AudioFormat.ENCODING_PCM_16BIT);
//
//        audioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, sampleRateInHz
//                , channelConfig, AudioFormat.ENCODING_PCM_16BIT, bufferSize,
//                AudioTrack.MODE_STREAM);
//        audioTrack.play();
//    }
//
//    //c传入音频数据
//    public void playTrack(byte[] buffer, int length) {
//        if (audioTrack != null && audioTrack.getPlayState() == AudioTrack.PLAYSTATE_PLAYING) {
//            audioTrack.write(buffer, 0, length);
//        } else {
//            audioTrack.pause();
//            audioTrack.release();
//        }
//    }

//    public void stop() {
//        if (audioTrack != null && audioTrack.getPlayState() == AudioTrack.PLAYSTATE_PLAYING) {
//            audioTrack.pause();
//            audioTrack.release();
//        }
//
//    }


}
