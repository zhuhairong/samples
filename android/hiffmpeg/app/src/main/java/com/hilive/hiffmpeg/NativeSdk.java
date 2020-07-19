package com.hilive.hiffmpeg;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Process;
import android.util.Log;
import android.view.Surface;

public class NativeSdk {
    private final static String TAG = "[hilive][NativeSdk]";
    private static NativeSdk mInstace = new NativeSdk();
    private HandlerThread mHandlerThread = null;
    private Handler mHandler = null;

    static {
        System.loadLibrary("mmavmedia");
        System.loadLibrary("native-lib");
    }

    public static NativeSdk getInstance() {
        return mInstace;
    }

    NativeSdk() {
        mHandlerThread = new HandlerThread("NativeSdk");
        mHandlerThread.setPriority(Process.THREAD_PRIORITY_BACKGROUND);
        mHandlerThread.start();
        mHandler = new Handler(mHandlerThread.getLooper());
    }

    public void runOnThread(final Runnable runnable) {
        mHandler.post(runnable);
    }

    public void reParseH264(final String filePath) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                Log.i(TAG, "ReParseH264 " + filePath);
                ReParseH264(filePath);
            }
        });
    }

    public void setEnableHardware(final boolean enable) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                Log.i(TAG, "SetEnableHardware");
                SetEnableHardware(enable);
            }
        });
    }

    public void setEnableMultiThread(final boolean enable) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                Log.i(TAG, "SetEnableMultiThread");
                SetEnableMultiThread(enable);
            }
        });
    }

    public void setEnableVideo(final boolean enable) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                Log.i(TAG, "SetEnableVideo");
                SetEnableVideo(enable);
            }
        });
    }

    public void setEnableAudio(final boolean enable) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                Log.i(TAG, "SetEnableAudio");
                SetEnableAudio(enable);
            }
        });
    }

    public void setEncodeParmas(int count, int width, int height, int fps, int bitrate) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                Log.i(TAG, "SetEncodeParmas");
                SetEncodeParmas(count, width, height, fps, bitrate);
            }
        });
    }

    public void setBaseDir(final String basedir) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                Log.i(TAG, "SetBaseDir");
                SetBaseDir(basedir);
            }
        });
    }

    public void testBenchMark() {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                TestBenchMark();
            }
        });
    }

    public void testNativeEncode(final String filePath) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                Log.i(TAG, "testNativeEncode " + filePath);
                TestNativeEncode(filePath);
            }
        });
    }

    public void testNativeDecode(final String filePath) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                Log.i(TAG, "testNativeDecode " + filePath);
                TestNativeDecode(filePath);
            }
        });
    }

    public void testFFEncode(final String filePath) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                Log.i(TAG, "testFFEncode " + filePath);
                TestFFEncode(filePath);
            }
        });
    }

    public void testFFDecode(final String filePath, final String outputDir) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                Log.i(TAG, "testFFDecode " + filePath);
                TestFFDecode(filePath, outputDir);
            }
        });
    }

    public void startPlay(final String filePath, final Surface surface, final int surfaceWidth, final int surfaceHeight) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                Log.i(TAG, "startPlay " + filePath);
                StartPlay(filePath, surface, surfaceWidth, surfaceHeight);
            }
        });
    }

    public void stopPlay() {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                Log.i(TAG, "stopPlay");
                StopPlay();
            }
        });
    }

    private native void ReParseH264(final String filePath);
    private native void SetEnableHardware(final boolean enable);
    private native void SetEnableMultiThread(final boolean enable);
    private native void SetEnableVideo(final boolean enable);
    private native void SetEnableAudio(final boolean enable);
    private native void SetEncodeParmas(final int count, final int width, final int height, final int fps, final int bitrate);
    private native void SetBaseDir(final String basedir);
    private native void TestBenchMark();
    private native void TestNativeEncode(final String filePath);
    private native void TestNativeDecode(final String filePath);
    private native void TestFFEncode(final String filePath);
    private native void TestFFDecode(final String filePath, final String outputDir);
    private native void StartPlay(final String filePath, final Surface surface, final int surfaceWidth, final int surfaceHeight);
    private native void StopPlay();
}
