#include <jni.h>
#include <string>
#include <unistd.h>
#include <android/log.h>
#include "native_sdk.h"
#include "media/utils.h"


static void ff_log_callback(void* ptr, int level, const char* format, va_list args) {
//  if (level > AV_LOG_VERBOSE) {
//    return;
//  }

    const uint32_t kBuffSize = 1024;
    char buff[kBuffSize + 1] = {0};

    vsnprintf(buff, kBuffSize, format, args);

    LOGD("[hilive][ffmpeg] [%p] [%d] %s", ptr, level, buff);
}


JavaVM* gJavaVM = NULL;

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOGD("%s %d", __FUNCTION__, __LINE__);
    JNIEnv* env = NULL;
    if (vm->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }

    gJavaVM = vm;

    av_log_set_level(AV_LOG_INFO);
    av_log_set_callback(ff_log_callback);

    srand((unsigned )time(NULL));
    return JNI_VERSION_1_6;
}

JNIEXPORT void JNI_OnUnload(JavaVM* vm, void* reserved) {
    LOGD("%s %d", __FUNCTION__, __LINE__);
}


extern "C" JNIEXPORT jlong JNICALL
Java_com_hilive_mediacodec_NativeSdk_nativeInit(
        JNIEnv* env,
        jobject /* this */) {
    return reinterpret_cast<uint64_t>(new NativeSdk());
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_hilive_mediacodec_NativeSdk_nativePlayStart(
        JNIEnv* env,
        jobject /* this */,
        jlong jctx,
        jstring jfilepath,
        jobject jsurface) {
    NativeSdk* sdk = reinterpret_cast<NativeSdk*>(jctx);
    if (!sdk) {
        return false;
    }

    std::string file_path;
    JNIHelper::Jstring2Str(env, jfilepath, file_path);

    sdk->PlayStart(ANativeWindow_fromSurface(env, jsurface), file_path);
    return true;
}

extern "C" JNIEXPORT void JNICALL
Java_com_hilive_mediacodec_NativeSdk_nativePlayUpdate(
        JNIEnv* env,
        jobject /* this */,
        jlong jctx) {
    NativeSdk* sdk = reinterpret_cast<NativeSdk*>(jctx);
    if (!sdk) {
        return;
    }

    sdk->PlayUpdate();
}

extern "C" JNIEXPORT void JNICALL
Java_com_hilive_mediacodec_NativeSdk_nativePlayStop(
        JNIEnv* env,
        jobject /* this */,
        jlong jctx) {
    NativeSdk* sdk = reinterpret_cast<NativeSdk*>(jctx);
    if (!sdk) {
        return;
    }

    sdk->PlayStop();
}

extern "C" JNIEXPORT void JNICALL
Java_com_hilive_mediacodec_NativeSdk_nativeUint(
        JNIEnv* env,
        jobject /* this */,
        jlong jctx) {
    NativeSdk* sdk = reinterpret_cast<NativeSdk*>(jctx);
    if (!sdk) {
        return;
    }

    delete sdk;
}