package com.hilive.nativecodec;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.hilive.nativecodec.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {
    private final static String TAG = "[hitest][java]";

    private final static int REQUEST_FFMPEG_DECODE = 100;

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("mmavmedia");
        System.loadLibrary("native-lib");
    }

    private ActivityMainBinding binding;
    private String baseDir = "";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        baseDir = this.getExternalCacheDir().getAbsolutePath() + "/temp/";

        Button btTestVideoEncode = findViewById(R.id.btTestVideoEncode);
        btTestVideoEncode.setOnClickListener(mClickListener);

        Button btTestVideoDecode = findViewById(R.id.btTestVideoDecode);
        btTestVideoDecode.setOnClickListener(mClickListener);

        Button btTestFfmpegEncode = findViewById(R.id.btTestFfmpegEncode);
        btTestFfmpegEncode.setOnClickListener(mClickListener);

        Button btTestFfmpegDecode = findViewById(R.id.btTestFfmpegDecode);
        btTestFfmpegDecode.setOnClickListener(mClickListener);

        Log.i(TAG, "os: " + Build.HARDWARE);
    }

    private View.OnClickListener mClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            switch (view.getId()) {
                case R.id.btTestVideoEncode: {
                    new Thread(new Runnable() {
                        @Override
                        public void run() {
                            String filePath = baseDir + "encode.h264";
                            testVideoEncode(filePath);
                        }
                    }).start();
                } break;
                case R.id.btTestVideoDecode: {
                    new Thread(new Runnable() {
                        @Override
                        public void run() {
                            String filePath = baseDir + "encode.h264";
                            testVideoDecode(filePath);
                        }
                    }).start();
                } break;
                case R.id.btTestAudioEncode: {
                    new Thread(new Runnable() {
                        @Override
                        public void run() {
                            String filePath = baseDir + "encode.aac";
                            testAudioEncode(filePath);
                        }
                    }).start();
                } break;
                case R.id.btTestAudioDecode: {
                    new Thread(new Runnable() {
                        @Override
                        public void run() {
                            String filePath = baseDir + "encode.aac";
                            testAudioDecode(filePath);
                        }
                    }).start();
                } break;
                case R.id.btTestFfmpegEncode: {
                    new Thread(new Runnable() {
                        @Override
                        public void run() {
                            String filePath = baseDir + "encode.h264";
                            testFfmpegEncode(filePath);
                        }
                    }).start();
                } break;
                case R.id.btTestFfmpegDecode: {
//                    pickFile(REQUEST_FFMPEG_DECODE);
                    String filePath = baseDir + "encode.h264";
                    testFfmpegDecode(filePath);
                }break;
                default:break;
            }
        }
    };

    private void pickFile(final int requestCodec) {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);//只显示可以打开的文件
        intent.setType("*/*");

        startActivityForResult(intent, requestCodec);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent intent) {
        super.onActivityResult(requestCode, resultCode, intent);

        Log.i(TAG, "onActivityResult, requestCode: " + requestCode + " resultCode: " + resultCode);
        if (resultCode != RESULT_OK || intent == null) {
            return;
        }

        Uri uri = intent.getData();
        String filePath = Utils.createTempPath(uri, this, "temp.mp4");
        if (filePath == null) {
            return;
        }

        Log.i(TAG, "onActivityResult, filePath: " + filePath);
        Log.i(TAG, "onActivityResult, " + intent.toString());
        Log.i(TAG, "onActivityResult, uri: " + uri);
        Log.i(TAG, "onActivityResult, path: " + uri.getPath());
        Log.i(TAG, "onActivityResult, scheme: " + uri.getScheme());

        switch (requestCode) {
            case REQUEST_FFMPEG_DECODE: {
                onTestFfmpegDecode(filePath);
            } break;
            default:break;
        }
    }

    private void onTestFfmpegDecode(final String filePath) {
        new Thread(new Runnable() {
            @Override
            public void run() {
//                String filePath = "/sdcard/tmp/douyin1.mp4";
//                            String filePath = baseDir + "encode.h264";
                testFfmpegDecode(filePath);
            }
        }).start();
    }

    public native void testVideoEncode(String filePath);
    public native void testVideoDecode(String filePath);
    public native void testAudioEncode(String filePath);
    public native void testAudioDecode(String filePath);
    public native void testFfmpegEncode(String filePath);
    public native void testFfmpegDecode(String filePath);
}