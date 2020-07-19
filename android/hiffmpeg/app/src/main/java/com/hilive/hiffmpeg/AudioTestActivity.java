package com.hilive.hiffmpeg;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.Switch;

import java.io.File;
import java.net.URISyntaxException;

public class AudioTestActivity extends AppCompatActivity {
    private static final String TAG = "[hilive][Test]";
    private String mBaseDir = "";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_audio_test);

        mBaseDir = this.getExternalCacheDir().getAbsolutePath() + "/";

        NativeSdk.getInstance().setEnableAudio(true);

        Button btAudioDecode = findViewById(R.id.btAudioDecode);
        btAudioDecode.setOnClickListener(mClickListener);
    }

    private View.OnClickListener mClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            updateConfig();

            switch (view.getId()) {
                case R.id.btAudioDecode: {
                    pickFile(1);
                    break;
                }
                default:break;
            }
        }
    };

    private void updateConfig() {
        Switch swHardware = findViewById(R.id.swHardware);
        Log.i(TAG, "updateConfig swHardware " + (swHardware.isChecked() ? "yes" : "no"));
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

        try {
            Uri uri = data.getData();
            String filePath = FileUtil.getPath(getApplicationContext(), uri);
            Log.i(TAG, "onActivityResult, requestCode: " + requestCode + " path: " + uri.getPath() + " host: " + uri.getHost() + " port: " + uri.getPort() + " encodedPath: " + uri.getEncodedPath() +
                    " gragment: " + uri.getFragment() + " scheme: " + uri.getScheme());

            if (filePath.contains(".")) {
                String ext = filePath.substring(filePath.lastIndexOf("."));
                Log.i(TAG, "filePath ext: " + ext);

                String tmpPath = Utils.createTempPath(uri, this, "temp" + ext, true);
                if (tmpPath == null) {
                    return;
                }

                NativeSdk.getInstance().testFFDecode(tmpPath, mBaseDir);
            }
        } catch (URISyntaxException e) {
        }
    }

    private void trans(final String filePath) {
        File file = new File(filePath);
        String name = file.getName();
        if (file.isFile()) {
            final String pcmPath = mBaseDir + "/test.pcm";
            final String aacPath = mBaseDir + "/test.m4a";
            Log.e(TAG, "打印路径：" + file.getPath() + "          " + file.getAbsolutePath());

            AudioCodec.getPCMFromAudio(file.getAbsolutePath(), pcmPath, new AudioCodec.AudioDecodeListener() {
                @Override
                public void decodeOver() {
                    Log.e(TAG, "音频解码完成" + pcmPath);
                    AudioCodec.PcmToAudio(pcmPath, aacPath, new AudioCodec.AudioDecodeListener() {
                        @Override
                        public void decodeOver() {
                            Log.e(TAG, "音频编码完成");
                        }

                        @Override
                        public void decodeFail() {

                        }
                    });
                }

                @Override
                public void decodeFail() {

                }
            });
        }
    }
}