package com.hubing.ffmpeg.hvideo;

import android.content.Context;
import android.graphics.PixelFormat;
import android.util.AttributeSet;
import android.view.Surface;
import android.view.SurfaceView;

import com.hubing.ffmpeg.FFmpegUtils;

/**
 * Created by hubing on 2017/10/9.
 */

public class HVideoView extends SurfaceView {

    private  String path ;

    public HVideoView(Context context) {
        super(context);
        init();
    }

    public HVideoView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public HVideoView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    private void init(){
        HVideoView.this.getHolder().setFormat(PixelFormat.RGBA_8888);
    }

    public void  player(String path){
        this.path = path;
        thread.start();
    }

    Thread thread = new Thread(new Runnable() {
        @Override
        public void run() {
            if(path!=null) {
                Surface surface = HVideoView.this.getHolder().getSurface();
                FFmpegUtils.getInstance().HPlayer(path, surface);
            }
        }
    });
}
