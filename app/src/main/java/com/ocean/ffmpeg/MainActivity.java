package com.ocean.ffmpeg;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);


    }

    public void player(View view) {
        MusicPlayer player = new MusicPlayer();
        player.play("rtmp://live.hkstv.hk.lxdns.com/live/hks");
//        player.play("rtmp://live.hkstv.hk.lxdns.com/live/hks/playlist.m3u8");
    }
}
