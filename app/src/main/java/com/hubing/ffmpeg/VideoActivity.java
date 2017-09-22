package com.hubing.ffmpeg;

import android.Manifest;
import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import java.io.File;

import permissions.dispatcher.NeedsPermission;
import permissions.dispatcher.RuntimePermissions;


@RuntimePermissions
public class VideoActivity extends AppCompatActivity {

    private VideoView video;
    final String input = new File(Environment.getExternalStorageDirectory(), "input.mp4").getAbsolutePath();
    final String  output= new File(Environment.getExternalStorageDirectory(), "output.yuv").getAbsolutePath();
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_video);


       // Button btn = (Button) findViewById(R.id.btn);
       video = (VideoView) findViewById(R.id.video);



    }

    @Override
    protected void onStart() {
        super.onStart();
        VideoActivityPermissionsDispatcher.playerWithPermissionCheck(VideoActivity.this);
    }
    @NeedsPermission({Manifest.permission.READ_EXTERNAL_STORAGE,Manifest.permission.WRITE_EXTERNAL_STORAGE})
    void player() {
        video.player(input);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           String[] permission,
                                           int[] grantResults){
        VideoActivityPermissionsDispatcher.onRequestPermissionsResult(VideoActivity.this, requestCode, grantResults);
    }



}