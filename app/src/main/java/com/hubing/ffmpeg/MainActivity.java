package com.hubing.ffmpeg;

import android.Manifest;
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

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("avcodec-56");
        System.loadLibrary("avdevice-56");
        System.loadLibrary("avfilter-5");
        System.loadLibrary("avformat-56");
        System.loadLibrary("avutil-54");
        System.loadLibrary("postproc-53");
        System.loadLibrary("swresample-1");
        System.loadLibrary("swscale-3");
        System.loadLibrary("native-lib");
    }

    final String input = new File(Environment.getExternalStorageDirectory(), "input.mp4").getAbsolutePath();
    final String  output= new File(Environment.getExternalStorageDirectory(), "output.yuv").getAbsolutePath();
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        TextView tv = (TextView) findViewById(R.id.sample_text);
        tv.setText(stringFromJNI());


        Button btn = (Button) findViewById(R.id.btn);
        btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                MainActivityPermissionsDispatcher.to2WithPermissionCheck(MainActivity.this,input,output);

            }
        });


        //mp4ToYuv(input,output);
    }

    @NeedsPermission({Manifest.permission.READ_EXTERNAL_STORAGE,Manifest.permission.WRITE_EXTERNAL_STORAGE})
    void to2(String input, String output) {
        if(new File(input).exists()){
            Log.d("ffmpeg","input exists:"+input);
            try {
                InputStream in = new FileInputStream(new File(input));
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            }
        }else {
            Log.d("ffmpeg","input  is not exists:"+input);
        }
        mp4ToYuv(input,output);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           String[] permission,
                                           int[] grantResults){
        MainActivityPermissionsDispatcher.onRequestPermissionsResult(this, requestCode, grantResults);
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
    public native void mp4ToYuv(String in,String out);
}
