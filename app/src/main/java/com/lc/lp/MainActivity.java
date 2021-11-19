package com.lc.lp;

import androidx.activity.result.ActivityResult;
import androidx.activity.result.ActivityResultCallback;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.annotation.NonNull;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;
import androidx.constraintlayout.widget.ConstraintLayout;

import android.Manifest;
import android.content.Intent;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.lc.lp.databinding.ActivityMainBinding;
import com.lc.lp.utils.CommonUtils;
import com.lc.lp.utils.GlideEngine;
import com.luck.picture.lib.PictureSelector;
import com.luck.picture.lib.config.PictureConfig;
import com.luck.picture.lib.config.PictureMimeType;
import com.luck.picture.lib.entity.LocalMedia;
import com.permissionx.guolindev.PermissionX;
import com.permissionx.guolindev.callback.RequestCallback;

import java.util.List;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "MainActivity";
    private ActivityMainBinding binding;
    private ActivityResultLauncher<Intent> launcherResult;

    private LCPlayer mLcPlayer;
    private float mVideoRatio;
    private float mVideoTotalSeconds;
    private boolean isEOF = false;

    public Handler handler = new Handler(new Handler.Callback() {
        @Override
        public boolean handleMessage(@NonNull Message msg) {

            Bundle data = msg.getData();
            float pts = data.getFloat("pts");
            String hmd = CommonUtils.second2HM((long) pts);
            binding.timeTv.setText(hmd);
            float v = pts / mVideoTotalSeconds * 1000f ;
            Log.i(TAG, "handleMessage: pts = " + pts);
            Log.i(TAG, "updateVideoPtsCallback: v = " + v);
            binding.seekBar.setProgress((int) v);

            LCPlayer.MediaPlayStatus playStatus = mLcPlayer.getPlayStatus();
//            Log.d(TAG, "handleMessage: playStatus = " + playStatus);
            if (playStatus == LCPlayer.MediaPlayStatus.MEDIA_PLAY_STATUS_PLAYING) {
                binding.playBtn.setImageResource(R.drawable.iocn_pause);
            } else if (playStatus == LCPlayer.MediaPlayStatus.MEDIA_PLAY_STATUS_PAUSE) {
                binding.playBtn.setImageResource(R.drawable.icon_play);
            }
            
            if (Math.abs(mVideoTotalSeconds - pts) <= 1) {
                Log.d(TAG, "handleMessage: 到达结尾了");
                binding.playBtn.setImageResource(R.drawable.icon_play);
                isEOF = true;
            }
            return false;
        }
    });

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        //检查读写权限
        checkPermission();

        mVideoRatio = 0;
        mVideoTotalSeconds = 0;

        binding.seekBar.bringToFront();
        binding.seekBar.setProgress(0);
        binding.seekBar.setOnSeekBarChangeListener(onSeekBarChangeListener);

        // 注册需要写在onCreate或Fragment onAttach里，否则会报java.lang.IllegalStateException异常
        launcherResult = createActivityResultLauncher();

        mLcPlayer = new LCPlayer(MainActivity.this);
        binding.surfaceView.setEGLContextClientVersion(3);
        binding.surfaceView.setRenderer(mLcPlayer);
        //仅在有数据的时候渲染
        binding.surfaceView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

        ActionBar actionBar = getSupportActionBar();
        if (actionBar != null) {
            actionBar.setDisplayOptions(androidx.appcompat.app.ActionBar.DISPLAY_SHOW_CUSTOM);
            actionBar.setCustomView(R.layout.layout_action_bar);

            TextView barText = (TextView) actionBar.getCustomView().findViewById(R.id.action_text);
            Button barBtn = (Button) actionBar.getCustomView().findViewById(R.id.action_media_button);
            barBtn.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    PermissionX.init(MainActivity.this)
                            .permissions(Manifest.permission.WRITE_EXTERNAL_STORAGE,
                                    Manifest.permission.READ_EXTERNAL_STORAGE)
                            .request(new RequestCallback() {
                                @Override
                                public void onResult(boolean allGranted, @NonNull List<String> grantedList, @NonNull List<String> deniedList) {
                                    if (allGranted) {
                                        startChooseMedia();
                                    } else {
                                        Toast.makeText(MainActivity.this, "权限拒绝", Toast.LENGTH_SHORT).show();
                                    }
                                }
                            });
                }
            });

        }

        binding.playBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (isEOF) {
                    isEOF = false;
                    mLcPlayer.seekVideoPlayer(0);
                } else if (mLcPlayer.getPlayStatus() == LCPlayer.MediaPlayStatus.MEDIA_PLAY_STATUS_PLAYING) {
                    mLcPlayer.pauseVideoPlayer();
                    binding.playBtn.setImageResource(R.drawable.icon_play);
                } else if (mLcPlayer.getPlayStatus() == LCPlayer.MediaPlayStatus.MEDIA_PLAY_STATUS_PAUSE) {
                    mLcPlayer.playVideoPlayer();
                    binding.playBtn.setImageResource(R.drawable.iocn_pause);
                }
            }
        });
    }

    @Override
    protected void onStart() {
        super.onStart();
        mLcPlayer.initVideoPlayer();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mLcPlayer.stopVideoPlayer();
    }

    /**
     * 选择媒体
     */
    private void startChooseMedia() {
        PictureSelector.create(MainActivity.this)
                .openGallery(PictureMimeType.ofVideo())
                .imageEngine(GlideEngine.createGlideEngine())
                .selectionMode(PictureConfig.SINGLE)
                .isPageStrategy(true, true)
                .maxSelectNum(1)
                .maxVideoSelectNum(1)
                .forResult(launcherResult);
    }

    /**
     * 创建一个ActivityResultLauncher
     *
     * @return
     */
    private ActivityResultLauncher<Intent> createActivityResultLauncher() {
        return registerForActivityResult(new ActivityResultContracts.StartActivityForResult(), new ActivityResultCallback<ActivityResult>() {
            @Override
            public void onActivityResult(ActivityResult result) {
                int resultCode = result.getResultCode();
                Log.d(TAG, "onActivityResult: resultCode = " + resultCode);
                if (resultCode == RESULT_OK) {
                    List<LocalMedia> selectList = PictureSelector.obtainMultipleResult(result.getData());
                    if (selectList != null && selectList.size() >= 1) {
                        String filePath = selectList.get(0).getRealPath();
                        Log.d(TAG, "onActivityResult: realPath = " + filePath);
                        mLcPlayer.startVideoPlayerWithPath(filePath);
                        mVideoRatio = mLcPlayer.getVideoSizeRatio();
                        mVideoTotalSeconds = mLcPlayer.getVideoTotalSeconds();
                        Log.d(TAG, "video ratio: " + mVideoRatio);
                        Log.d(TAG, "video total seconds: " + mVideoTotalSeconds);
                        String hmd = CommonUtils.second2HM((long) mVideoTotalSeconds);
                        Log.d(TAG, "HMD = " + hmd);
                        binding.totalTimeTv.setText(hmd);
                    }

                }

                updateVideoSize();
            }
        });
    }

    /**
     * 权限申请
     */
    private void checkPermission() {
        PermissionX.init(MainActivity.this)
                .permissions(Manifest.permission.WRITE_EXTERNAL_STORAGE,
                        Manifest.permission.READ_EXTERNAL_STORAGE)
                .request(new RequestCallback() {
                    @Override
                    public void onResult(boolean allGranted, @NonNull List<String> grantedList, @NonNull List<String> deniedList) {
                        if (allGranted) {
                            Toast.makeText(MainActivity.this, "权限同意了", Toast.LENGTH_SHORT).show();
                        } else {
                            Toast.makeText(MainActivity.this, "权限拒绝", Toast.LENGTH_SHORT).show();
                        }
                    }
                });
    }

    public void updateVideoRenderCallback() {
        binding.surfaceView.requestRender();
    }

    public void updateVideoPtsCallback(float pts) {
        //需要转换到子线程去更新UI   否则会崩溃
        Message message = Message.obtain();
        Bundle bundle = new Bundle();
        bundle.putFloat("pts", pts);
        message.setData(bundle);
        handler.sendMessage(message);
    }

    private void updateVideoSize() {

        WindowManager manager = this.getWindowManager();
        DisplayMetrics outMetrics = new DisplayMetrics();
        manager.getDefaultDisplay().getMetrics(outMetrics);
        int width = outMetrics.widthPixels;
        int height = outMetrics.heightPixels;

        int topMargin = 0;
        if (mVideoRatio > 1) {
            topMargin = 200;
        }

        ConstraintLayout.LayoutParams layoutParams = (ConstraintLayout.LayoutParams) binding.surfaceView.getLayoutParams();
        layoutParams.topMargin = topMargin;
        layoutParams.leftMargin = 0;
        layoutParams.rightMargin = 0;
        layoutParams.width = width;
        layoutParams.height = (int) (width / mVideoRatio);

        binding.surfaceView.setLayoutParams(layoutParams);
    }

    private SeekBar.OnSeekBarChangeListener onSeekBarChangeListener = new SeekBar.OnSeekBarChangeListener() {
        @Override
        public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
//            Log.d(TAG, "当前进度: " + progress + ",isFromUser = " + fromUser);
//            if ((progress % 10 == 0) && fromUser) {
//                onSeekingVideo(progress);
//            }
        }

        @Override
        public void onStartTrackingTouch(SeekBar seekBar) {

        }

        @Override
        public void onStopTrackingTouch(SeekBar seekBar) {
            int currentValue = seekBar.getProgress();
            Log.d(TAG, "seek end: " + currentValue);
            onSeekingVideo(currentValue);
        }
    };

    private void onSeekingVideo(float aValue) {
        float currSliderRatio = aValue / 1000.0f;
        float seekingTime = currSliderRatio * mVideoTotalSeconds;

        if (seekingTime > mVideoTotalSeconds) {
            seekingTime = mVideoTotalSeconds;
        }

        Log.d(TAG, "Seeking Time: " + seekingTime);
        mLcPlayer.seekVideoPlayer(seekingTime);
    }

}