package com.hubing.ffmpeg;

import android.content.Context;
import android.graphics.PixelFormat;
import android.util.AttributeSet;
import android.view.Surface;
import android.view.SurfaceView;
import android.view.accessibility.AccessibilityEvent;

/**
 * Created by hubing on 2017/9/21.
 */

public class VideoView extends SurfaceView{
    private  String path ;

    public VideoView(Context context) {
        super(context);
    }

    public VideoView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public VideoView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    private void init(){
        VideoView.this.getHolder().setFormat(PixelFormat.RGB_888);
    }

    public void  player(String path){
        this.path = path;
        thread.start();
        }

    Thread thread = new Thread(new Runnable() {
        @Override
        public void run() {
            if(path!=null) {
                Surface surface = VideoView.this.getHolder().getSurface();
                FFmpegUtils.getInstance().player(path, surface);
            }
        }
    });
}
