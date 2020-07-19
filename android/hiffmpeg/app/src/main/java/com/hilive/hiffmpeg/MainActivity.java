package com.hilive.hiffmpeg;

import androidx.appcompat.app.AppCompatActivity;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import com.hilive.hiffmpeg.databinding.ActivityMainBinding;

import java.util.ArrayList;
import java.util.List;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "[hilive][main]";

    private ActivityMainBinding binding;
    private final int REQUEST_CODE_PERMISSION = 100;
    private final String[] ALL_PERMISSIONS = {
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.INTERNET,
            Manifest.permission.CAMERA
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        Button btAudioTest = findViewById(R.id.btAudioTest);
        btAudioTest.setOnClickListener(mClickListener);

        Button btVideoTest = findViewById(R.id.btVideoTest);
        btVideoTest.setOnClickListener(mClickListener);

        Button btPlay = findViewById(R.id.btPlay);
        btPlay.setOnClickListener(mClickListener);

        checkPermissions();
    }

    private View.OnClickListener mClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            switch (view.getId()) {
                case R.id.btAudioTest:
                    startActivity(new Intent(getApplicationContext(), AudioTestActivity.class));
                    break;
                case R.id.btVideoTest:
                    startActivity(new Intent(getApplicationContext(), VideoTestActivity.class));
                    break;
                case R.id.btPlay:
                    startActivity(new Intent(getApplicationContext(), PlayActivity.class));
                    break;
                default:break;
            }
        }
    };

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    private void checkPermissions() {
        if (Build.VERSION.SDK_INT >= 23) {
            List<String> permissions = new ArrayList<>();
            for (String str : ALL_PERMISSIONS) {
                if (this.checkSelfPermission(str) != PackageManager.PERMISSION_GRANTED) {
                    permissions.add(str);
                    Log.i(TAG, "checkPermissions fail, " + str);
                }
            }

            if (permissions.size() > 0) {
                String[] ps = new String[permissions.size()];
                permissions.toArray(ps);
                this.requestPermissions(ps, REQUEST_CODE_PERMISSION);
            }
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           String[] permission,
                                           int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permission, grantResults);

        Log.i(TAG, "onRequestPermissionsResult, code: " + requestCode);

        for (String p : permission) {
            Log.i(TAG, "onRequestPermissionsResult, permission: " + p);
        }

        int grantCount = 0;
        for (int g : grantResults) {
            Log.i(TAG, "onRequestPermissionsResult, grantResults: " + g);
            if (g == PackageManager.PERMISSION_GRANTED) {
                ++ grantCount;
            }
        }

        if (grantCount != permission.length) {
            Toast.makeText(this, getString(R.string.permission_tip), Toast.LENGTH_LONG).show();
        }
    }
}