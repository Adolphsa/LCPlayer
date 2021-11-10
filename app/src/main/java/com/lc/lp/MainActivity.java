package com.lc.lp;

import androidx.activity.result.ActivityResult;
import androidx.activity.result.ActivityResultCallback;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.annotation.NonNull;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;

import android.Manifest;
import android.content.Intent;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.lc.lp.databinding.ActivityMainBinding;
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

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        //检查读写权限
        checkPermission();

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
                    }

                }
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

}