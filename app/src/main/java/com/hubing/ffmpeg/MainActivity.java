package com.hubing.ffmpeg;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Environment;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStream;

import permissions.dispatcher.NeedsPermission;
import permissions.dispatcher.RuntimePermissions;


@RuntimePermissions
public class MainActivity extends AppCompatActivity {


    final String input = new File(Environment.getExternalStorageDirectory(), "input.mp4").getAbsolutePath();
    final String  output= new File(Environment.getExternalStorageDirectory(), "output.yuv").getAbsolutePath();
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        TextView tv = (TextView) findViewById(R.id.sample_text);
        tv.setText(FFmpegUtils.getInstance().stringFromJNI());


        Button btn = (Button) findViewById(R.id.btn);
        Button video = (Button) findViewById(R.id.video);
        btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                MainActivityPermissionsDispatcher.mp42YuvWithPermissionCheck(MainActivity.this,input,output);

            }
        });
        video.setOnClickListener(new View.OnClickListener(){

            @Override
            public void onClick(View view) {
                startActivity(new Intent(MainActivity.this,com.hubing.ffmpeg.VideoActivity.class));
            }
        });

    }

    @NeedsPermission({Manifest.permission.READ_EXTERNAL_STORAGE,Manifest.permission.WRITE_EXTERNAL_STORAGE})
    void mp42Yuv(String input, String output) {
        FFmpegUtils.getInstance().mp4ToYuv(input,output);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           String[] permission,
                                           int[] grantResults){
        MainActivityPermissionsDispatcher.onRequestPermissionsResult(this, requestCode, grantResults);
    }



}
