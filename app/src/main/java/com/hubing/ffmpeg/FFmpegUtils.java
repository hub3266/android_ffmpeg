package com.hubing.ffmpeg;

import android.view.Surface;

/**
 * Created by hubing on 2017/9/21.
 */

public class FFmpegUtils {
    private static FFmpegUtils instance =null;
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

    public static FFmpegUtils getInstance() {
        if(instance == null){
            instance = new FFmpegUtils();
        }
        return instance;
    }

    public native String stringFromJNI();
    public native void mp4ToYuv(String in,String out);
    public native void player(String in,Surface surface);
    public native void mp3Player(String in,AudioPlayer audioPlayer);
}
