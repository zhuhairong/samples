package com.hilive.hiffmpeg;

import androidx.appcompat.app.AppCompatActivity;

import android.annotation.*;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;

public class PlayActivity extends AppCompatActivity implements SurfaceHolder.Callback {
    private final static String TAG = "[hilive][java]";

    private Surface mSurface = null;
    private int mSurfaceWidth = 0;
    private int mSurfaceHeight = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_play);

        SurfaceView svPlay = findViewById(R.id.svPlay);
        svPlay.getHolder().addCallback(this);

        Button btStart = findViewById(R.id.btStart);
        btStart.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
//                Uri uri = Uri.parse("android.resource://" + getPackageName() + "/" + R.raw.guanggao);
//                String filePath = Utils.createTempPath(uri, getApplicationContext(), "temp.mp4");
//                if (filePath == null) {
//                    return;
//                }
//
//                NativeSdk.getInstance().startPlay(filePath, mSurface, mSurfaceWidth, mSurfaceHeight);
            }
        });

        Button btStop = findViewById(R.id.btStop);
        btStop.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                NativeSdk.getInstance().stopPlay();
            }
        });
    }

    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {
        mSurface = surfaceHolder.getSurface();
        Log.i(TAG, "surfaceCreated");
    }

    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int format, int width, int height) {
        mSurface = surfaceHolder.getSurface();
        mSurfaceWidth = width;
        mSurfaceHeight = height;
        Log.i(TAG, "surfaceChanged, format: " + format + " width: " + width + " height: " + height);
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {
        Log.i(TAG, "surfaceDestroyed");
    }
}