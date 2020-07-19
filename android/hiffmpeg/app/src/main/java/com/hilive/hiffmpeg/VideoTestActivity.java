package com.hilive.hiffmpeg;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.Switch;

import java.util.ArrayList;

public class VideoTestActivity extends AppCompatActivity {
    private static final String TAG = "[hilive][Test]";
    private String mBaseDir = "";
    private String sampleH264 = "/sdcard/tmp/h.h264";
    private String sampleMp4 = "/sdcard/tmp/m.mp4";
    ArrayList<String> h264List = new ArrayList<>();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_video_test);

        mBaseDir = this.getExternalCacheDir().getAbsolutePath() + "/temp/";

        NativeSdk.getInstance().setEncodeParmas(1000, 720, 1280, 30, 3000000);
        NativeSdk.getInstance().setBaseDir(mBaseDir);
        NativeSdk.getInstance().setEnableVideo(true);

        Button btJavaEncode = findViewById(R.id.btJavaEncode);
        btJavaEncode.setOnClickListener(mClickListener);

        Button btJavaDecode = findViewById(R.id.btJavaDecode);
        btJavaDecode.setOnClickListener(mClickListener);

        Button btNativeEncode = findViewById(R.id.btNativeEncode);
        btNativeEncode.setOnClickListener(mClickListener);

        Button btNativeDecode = findViewById(R.id.btNativeDecode);
        btNativeDecode.setOnClickListener(mClickListener);

        Button btFFEncode = findViewById(R.id.btFfEncode);
        btFFEncode.setOnClickListener(mClickListener);

        Button btFFDecode = findViewById(R.id.btFfDecode);
        btFFDecode.setOnClickListener(mClickListener);

//        {
//            Uri uri = Uri.parse("android.resource://" + getPackageName() + "/" + R.raw.h);
//            sampleH264 = Utils.createTempPath(uri, getApplicationContext(), "temp.h264");
//            if (sampleH264 == null) {
//                Log.i(TAG, "createTempPath " + sampleH264 + " fail");
//                return;
//            }
//        }
//
//        NativeSdk.getInstance().reParseH264(sampleH264);
//
//        {
//            Uri uri = Uri.parse("android.resource://" + getPackageName() + "/" + R.raw.m);
//            sampleMp4 = Utils.createTempPath(uri, getApplicationContext(), "temp.mp4");
//            if (sampleMp4 == null) {
//                Log.i(TAG, "createTempPath " + sampleMp4 + " fail");
//                return;
//            }
//        }
    }

    private View.OnClickListener mClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            updateConfig();

            switch (view.getId()) {
                case R.id.btNativeEncode: {
                    String filePath = mBaseDir + "encode.h264";
                    NativeSdk.getInstance().testNativeEncode(filePath);
                    break;
                }
                case R.id.btFfEncode: {
                    String filePath = mBaseDir + "encode.h264";
                    NativeSdk.getInstance().testFFEncode(filePath);
                    break;
                }
                case R.id.btNativeDecode: {
                    String filePath = mBaseDir + "encode.h264";
                    NativeSdk.getInstance().reParseH264(filePath);
                    NativeSdk.getInstance().testNativeDecode(filePath);
                    break;
                }
                case R.id.btJavaDecode:
                case R.id.btFfDecode: {
                    pickFile(view.getId());
                    break;
                }
                default:break;
            }
        }
    };

    private void updateConfig() {
        Switch swHardware = findViewById(R.id.swHardware);
        NativeSdk.getInstance().setEnableHardware(swHardware.isChecked());

        Switch swMultiThread = findViewById(R.id.swMultThread);
        NativeSdk.getInstance().setEnableMultiThread(swMultiThread.isChecked());
    }

    private void pickFile(final int id) {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);//只显示可以打开的文件
        intent.setType("*/*");

        this.startActivityForResult(intent, id);
    }

    @Override
    protected void onActivityResult(final int requestCode, final int resultCode, final Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (resultCode != RESULT_OK || data == null) {
            return;
        }

        Uri uri = data.getData();

        Log.i(TAG, "onActivityResult, requestCode: " + requestCode + " path: " + uri.getPath() + " host: " + uri.getHost() + " port: " + uri.getPort() + " encodedPath: " + uri.getEncodedPath() +
                " gragment: " + uri.getFragment() + " scheme: " + uri.getScheme());

        switch (requestCode) {
            case R.id.btJavaDecode: {
                NativeSdk.getInstance().runOnThread(new Runnable() {
                    @Override
                    public void run() {
                        String filePath = Utils.createTempPath(uri, getApplicationContext(), "temp.mp4", false);
                        if (filePath == null) {
                            return;
                        }

                        H264Decoder decoder = new H264Decoder();
                        if (!decoder.init(filePath)) {
                            return;
                        }

                        decoder.decodeLoop();
                        decoder.uint();
                    }
                });
                break;
            }
            case R.id.btNativeDecode: {
                String filePath = Utils.createTempPath(uri, this, "temp.h264", false);
                if (filePath == null) {
                    return;
                }

                if (!h264List.contains(filePath)) {
                    NativeSdk.getInstance().reParseH264(filePath);
                    h264List.add(filePath);
                    return;
                }

                NativeSdk.getInstance().testNativeDecode(filePath);
                break;
            }
            case R.id.btFfDecode: {
                String filePath = Utils.createTempPath(uri, this, "temp.mp4", true);
                if (filePath == null) {
                    return;
                }

                NativeSdk.getInstance().testFFDecode(filePath, mBaseDir);
                break;
            }
            default:break;
        }
    }
}