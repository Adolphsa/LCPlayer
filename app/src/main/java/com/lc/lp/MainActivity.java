package com.lc.lp;

import androidx.appcompat.app.AppCompatActivity;

import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.widget.TextView;

import com.lc.lp.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    private ActivityMainBinding binding;


    private LCPlayer mLcPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        mLcPlayer = new LCPlayer(MainActivity.this);
        binding.surfaceView.setEGLContextClientVersion(3);
        binding.surfaceView.setRenderer(mLcPlayer);
        //仅在有数据的时候渲染
        binding.surfaceView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }
}