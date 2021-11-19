package com.lc.lp;

import static com.lc.lp.LCPlayer.MediaPlayStatus.MEDIA_PLAY_STATUS_PLAYING;

import android.content.Context;
import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.util.Log;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * Created by lucas on 2021/11/8.
 */
public class LCPlayer implements GLSurfaceView.Renderer {
    private static final String TAG = "LCPlayer";

    static {
        System.loadLibrary("LCPlayer");
    }

    enum MediaPlayStatus
    {
        MEDIA_PLAY_STATUS_PLAYING,
        MEDIA_PLAY_STATUS_PAUSE,
        MEDIA_PLAY_STATUS_SEEK,
        MEDIA_PLAY_STATUS_STOP
    }

    private Context mGLContext;

    LCPlayer(Context ctx) {
        mGLContext = ctx;
    }

    public native String stringFromJNI();

    private native void ndkInitVideoPlayer();

    private native void ndkStartPlayerWithFile(String fileName);

    private native int ndkGetPlayStatus();

    private native void ndkPauseVideoPlay();

    private native void ndkPlayVideoPlay();

    private native void ndkStopVideoPlayer();

    private native float ndkGetVideoSizeRatio();

    private native float ndkGetVideoTotalSeconds();

    private native void ndkSeekMedia(float nPos);

    private native void ndkInitGL(AssetManager assetManager);

    private native void ndkResizeGL(int width, int height);

    private native void ndkPaintGL();

    @Override
    public void onSurfaceCreated(GL10 gl10, EGLConfig eglConfig) {
        AssetManager assetManager = mGLContext.getAssets();
        ndkInitGL(assetManager);
    }

    @Override
    public void onSurfaceChanged(GL10 gl10, int width, int height) {
        ndkResizeGL(width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl10) {
        ndkPaintGL();
    }

    public void initVideoPlayer() {
        ndkInitVideoPlayer();
    }

    public void startVideoPlayerWithPath(String filePath) {
        ndkStartPlayerWithFile(filePath);
    }

    public void pauseVideoPlayer() {
        ndkPauseVideoPlay();
    }

    public void playVideoPlayer() {
        ndkPlayVideoPlay();
    }

    public void stopVideoPlayer() {
        ndkStopVideoPlayer();
    }

    public MediaPlayStatus getPlayStatus() {
        MediaPlayStatus mediaPlayStatus = MediaPlayStatus.MEDIA_PLAY_STATUS_STOP;
        int ndkPlayStatus = ndkGetPlayStatus();
        switch (ndkPlayStatus) {
            case 0:
                mediaPlayStatus = MediaPlayStatus.MEDIA_PLAY_STATUS_PLAYING;
                break;
            case 1:
                mediaPlayStatus = MediaPlayStatus.MEDIA_PLAY_STATUS_PAUSE;
                break;
            case 2:
                mediaPlayStatus = MediaPlayStatus.MEDIA_PLAY_STATUS_SEEK;
                break;
            case 3:
                mediaPlayStatus = MediaPlayStatus.MEDIA_PLAY_STATUS_STOP;
                break;
        }
        return mediaPlayStatus;
    }

    public float getVideoSizeRatio() {
        float ratio = ndkGetVideoSizeRatio();
        return ratio;
    }

    public float getVideoTotalSeconds() {
        float totalSeconds = ndkGetVideoTotalSeconds();
        return totalSeconds;
    }

    public void seekVideoPlayer(float pos) {
        ndkSeekMedia(pos);
    }

    public void OnVideoRenderCallback() {
        MainActivity activity = (MainActivity)mGLContext;
        if(activity != null) {
            activity.updateVideoRenderCallback();
        }
    }

    public void OnPtsCallback(float pts) {
//        Log.d(TAG, "OnPtsCallback: " + pts);
        MainActivity activity = (MainActivity)mGLContext;
        if(activity != null) {
            activity.updateVideoPtsCallback(pts);
        }
    }
}
