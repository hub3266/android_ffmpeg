package com.hubing.ffmpeg.hvideo;

import android.Manifest;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;

import com.hubing.ffmpeg.R;
import com.hubing.ffmpeg.VideoView;

import java.io.File;

import permissions.dispatcher.NeedsPermission;
import permissions.dispatcher.RuntimePermissions;


@RuntimePermissions
public class HVideoActivity extends AppCompatActivity {
    private String input = "rtmp://live.hkstv.hk.lxdns.com/live/hks";
    //final String input = new File(Environment.getExternalStorageDirectory(), "input.mp4").getAbsolutePath();
    private HVideoView video;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_hvideo);


        Button btn = (Button) findViewById(R.id.btn);
        video = (HVideoView) findViewById(R.id.HVideo);
        btn.setOnClickListener(new View.OnClickListener(){

            @Override
            public void onClick(View view) {
                HVideoActivityPermissionsDispatcher.playerWithPermissionCheck(HVideoActivity.this);
            }
        });


    }




    @NeedsPermission({Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE,})
    void player() {
        video.player(input);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           String[] permission,
                                           int[] grantResults) {
        HVideoActivityPermissionsDispatcher.onRequestPermissionsResult(HVideoActivity.this, requestCode, grantResults);
    }



}
