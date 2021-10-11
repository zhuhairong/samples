package com.hilive.mediacodec;

import android.content.Context;
import android.view.Surface;

public class NativeSdk {
    static {
        System.loadLibrary("mmavmedia");
        System.loadLibrary("native-lib");
    }

    private final static String TAG = "[hilive][NativeSdk]";
    private static NativeSdk mInstace = new NativeSdk();

    public static NativeSdk getInstance() {
        return mInstace;
    }

    public native long PlayInit();
    public native boolean PlayStart(final long ctx, final String filePath, final Surface surface);
    public native void PlayUpdate(final long ctx);
    public native void PlayStop(final long ctx);
    public native void PlayUint(final long ctx);


}
