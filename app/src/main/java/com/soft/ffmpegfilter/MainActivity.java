package com.soft.ffmpegfilter;

import android.Manifest;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;

import com.soft.function.BaseActivity;
import com.soft.function.FunctionControl;

import java.io.File;

public class MainActivity extends BaseActivity {

    public static final String TAG = "MainActivity";
    private final static String PATH = Environment.getExternalStorageDirectory() + File.separator
            + "filefilm" + File.separator + "mediatest1.mp4";

    private Button mStartPlay;
    private Button mPausePlay;
    private SurfaceView m_surface_view;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        requestRunTimePermission(new String[]{
                        Manifest.permission.INTERNET,
                        Manifest.permission.READ_EXTERNAL_STORAGE,
                        Manifest.permission.WRITE_EXTERNAL_STORAGE},
                null);

        initCommon();
        initSurfaceView();
        FunctionControl.getInstance().preparePlayer(PATH);
    }

    private void initCommon() {
        mStartPlay = findViewById(R.id.start_play);
        mStartPlay.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                FunctionControl.getInstance().startPlayer();
            }
        });

        mPausePlay = findViewById(R.id.pause_play);
        mPausePlay.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                FunctionControl.getInstance().stopPlayer();
            }
        });
    }

    private void initSurfaceView() {
        m_surface_view = findViewById(R.id.surface_view);
        m_surface_view.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                Log.d(TAG, "surfaceCreated: ");
                FunctionControl.getInstance().saveAssetManager(getAssets());
                FunctionControl.getInstance().setSurface(holder.getSurface());
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                Log.d(TAG, "surfaceChanged: ");
                FunctionControl.getInstance().setSurfaceSize(width, height);
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                Log.d(TAG, "surfaceDestroyed: ");
                FunctionControl.getInstance().releaseResources();
            }
        });
    }
}
