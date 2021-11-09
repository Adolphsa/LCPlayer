package com.lc.lp;

import android.content.Context;
import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * Created by lucas on 2021/11/8.
 */
public class LCPlayer implements GLSurfaceView.Renderer {

    static {
        System.loadLibrary("LCPlayer");
    }

    private Context mGLContext;

    LCPlayer(Context ctx) {
        mGLContext = ctx;
    }
    public native String stringFromJNI();

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
}
