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
    private boolean mInited = false;
    private long mContext = 0;

    public static NativeSdk getInstance() {
        return mInstace;
    }

    synchronized public boolean init(final Context context) {
        if (mContext == 0) {
            mContext = nativeInit();
            mInited = mContext > 0 ? true : false;
        }

        return mInited;
    }

    synchronized public void playStart(final String filePath, final Surface surface) {
        if (!mInited) {
            return;
        }

        nativePlayStart(mContext, filePath, surface);
    }

    synchronized public void playUpdate() {
        if (!mInited) {
            return;
        }

        nativePlayUpdate(mContext);
    }

    synchronized public void playStop() {
        if (!mInited) {
            return;
        }

        nativePlayStop(mContext);
    }

    synchronized public void uint() {
        if (mContext != 0) {
            nativeUint(mContext);
            mContext = 0;
        }

        mInited = false;
    }

    private native long nativeInit();
    private native boolean nativePlayStart(final long ctx, final String filePath, final Surface surface);
    private native void nativePlayUpdate(final long ctx);
    private native void nativePlayStop(final long ctx);
    private native void nativeUint(final long ctx);
}
