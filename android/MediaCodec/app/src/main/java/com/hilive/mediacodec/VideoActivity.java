package com.hilive.mediacodec;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;

import java.net.URISyntaxException;

public class VideoActivity extends AppCompatActivity implements SurfaceHolder.Callback {
    private final static String TAG = "[hilive][video]";
    private Surface mSurface = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_video);

        SurfaceView svPlay = findViewById(R.id.svPlay);
        svPlay.getHolder().addCallback(this);

        Button btStart = findViewById(R.id.btStart);
        btStart.setOnClickListener(mClickListener);

        Button btStop = findViewById(R.id.btStop);
        btStop.setOnClickListener(mClickListener);
    }

    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {
        mSurface = surfaceHolder.getSurface();
        Log.i(TAG, "surfaceCreated");
    }

    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int format, int width, int height) {
        mSurface = surfaceHolder.getSurface();
        Log.i(TAG, "surfaceChanged, format: " + format + " width: " + width + " height: " + height);
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {
        Log.i(TAG, "surfaceDestroyed");
    }

    private View.OnClickListener mClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            switch (v.getId()) {
                case R.id.btStart:
                    pickFile(v.getId());
                    break;
                case R.id.btStop:
                    NativeSdk.getInstance().playStop();
                    break;
                default:break;
            }
        }
    };

    private void pickFile(final int requestCodec) {
        Intent intent = new Intent(this, SettingActivity.class);
        intent.putExtra("requestCode", requestCodec);
        startActivityForResult(intent, 0);
    }

    @Override
    protected void onActivityResult(final int requestCode, final int resultCode, final Intent intent) {
        super.onActivityResult(requestCode, resultCode, intent);
        if (resultCode != RESULT_OK || intent == null) {
            Log.i(TAG, "onActivityResult fail");
            return;
        }

        try {
            int code = intent.getIntExtra("requestCode", 0);
            Uri uri = intent.getData();
            String filePath = FileUtil.getPath(getApplicationContext(), uri);
            Log.i(TAG, "onActivityResult, code: " + code + " path: " + uri.getPath() + " host: " + uri.getHost() + " encodedPath: " + uri.getEncodedPath() +
                    " isAbsolute: " + uri.isAbsolute() + " isRelative: " + uri.isRelative());

            NativeSdk.getInstance().playStart(filePath, mSurface);
        } catch (URISyntaxException e) {
            Log.e(TAG, "URISyntaxException: " + e.getMessage());
            e.printStackTrace();
        }
    }
}