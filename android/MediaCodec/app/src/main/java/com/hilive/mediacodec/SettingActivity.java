package com.hilive.mediacodec;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

public class SettingActivity extends AppCompatActivity {
    private static final String TAG = "[hilive][settings]";

    private int codeId = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_setting);

        codeId = getIntent().getIntExtra("requestCode", 0);

        Log.i(TAG, "requestCode: " + codeId);

        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);//只显示可以打开的文件
        intent.setType("*/*");

//        this.startActivityForResult(intent, 0);
        this.startActivityForResult(intent, 0);
    }

    @Override
    protected void onActivityResult(final int requestCode, final int resultCode, final Intent intent) {
        super.onActivityResult(requestCode, resultCode, intent);

        intent.putExtra("requestCode", codeId);
        setResult(resultCode, intent);
        finish();
    }
}