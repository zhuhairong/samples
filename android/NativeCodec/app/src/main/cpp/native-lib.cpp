#include <jni.h>
#include <string>
#include "utils.h"
#include "jni_helper.h"
#include "video_decode.h"
#include "video_encode.h"
#include "audio_decode.h"
#include "audio_encode.h"
#include "ff_decoder.h"

static void ff_log_callback(void* ptr, int level, const char* format, va_list args) {
  const uint32_t kBuffSize = 1024;
  char buff[kBuffSize + 1] = {0};

  vsnprintf(buff, kBuffSize, format, args);

  LOGD("[hitest] [%p] [%d] %s", ptr, level, buff);
}

void FillData(void* data, uint32_t size) {
  uint32_t count = size / sizeof(uint32_t);
  uint32_t* ptr = (uint32_t*)data;
  uint32_t r = 0;
  uint32_t c = 0;
  for (uint32_t i = 0; i < count; ++ i) {
    c = r = rand() % RAND_MAX;
    c = r & 0xffffff00;
    ptr[i] = c;
  }
}

extern "C" JNIEXPORT void JNICALL
Java_com_hilive_nativecodec_MainActivity_testVideoEncode(
        JNIEnv* env, jobject thiz, jstring filePath) {
  uint64_t start_stamp = GetTickCount();
  uint32_t count = 0;

  do {
    srand((unsigned )time(NULL));

    std::string file_path;
    JNIHelper::Jstring2Str(env, filePath, file_path);
    CreateParentPath(file_path);

    LOGD("%s %d filePath: %s", __FUNCTION__, __LINE__, file_path.c_str());

    uint32_t width = 720;
    uint32_t height = 1280;
    uint32_t fps = 30;
    uint32_t bitrate = 3000000;
    float duration = AV_TIME_BASE / fps;
    VideoEncode encode;
    if (!encode.Init(width, height, fps, bitrate, file_path)) {
      break;
    }

    float pts = 0;
    uint32_t size = width * height * 3 / 2;
    void* data = malloc(size);
    LOGD("testVideoEncode size: %u", size);
    FillData(data, size);
    while (++ count < 300 && encode.Encode(data, size, pts)) {
      pts += duration;
      FillData(data, size);
    }
    free(data);
    encode.Flush();
    encode.Uint();
  } while (false);

  uint64_t end_stamp = GetTickCount();
  uint64_t spent = end_stamp - start_stamp;

  LOGD("%s %d spent: %llu count: %u avg: %llu", __FUNCTION__, __LINE__, spent, count, spent / count);
}

extern "C" JNIEXPORT void JNICALL
Java_com_hilive_nativecodec_MainActivity_testVideoDecode(
        JNIEnv* env,
        jobject thiz, jstring filePath) {
  uint64_t start_stamp = GetTickCount();
  do {
    std::string file_path;
    JNIHelper::Jstring2Str(env, filePath, file_path);

    MediaDecode decoder;
    if (!decoder.Init(file_path)) {
      break;
    }

    while (decoder.Progress()) {}
    decoder.Flush();
    decoder.Uint();
  } while (false);

  uint64_t end_stamp = GetTickCount();
  uint64_t spent = end_stamp - start_stamp;
  LOGD("%s %d spent: %llu", __FUNCTION__, __LINE__, spent);
}

extern "C" JNIEXPORT void JNICALL
Java_com_hilive_nativecodec_MainActivity_testAudioEncode(
        JNIEnv* env,
        jobject thiz, jstring filePath) {
}

extern "C" JNIEXPORT void JNICALL
Java_com_hilive_nativecodec_MainActivity_testAudioDecode(
        JNIEnv* env,
        jobject thiz, jstring filePath) {
}


extern "C" JNIEXPORT void JNICALL
Java_com_hilive_nativecodec_MainActivity_testFfmpegEncode(
        JNIEnv* env,
        jobject thiz, jstring filePath) {
}

extern "C" JNIEXPORT void JNICALL
Java_com_hilive_nativecodec_MainActivity_testFfmpegDecode(
        JNIEnv* env,
        jobject thiz, jstring filePath) {
  std::string file_path;
  JNIHelper::Jstring2Str(env, filePath, file_path);

  do {
    FFDecoder decoder;
    if (!decoder.Init(file_path.c_str(), true, false)) {
      break;
    }

    while (decoder.Progress()) {}
    decoder.Flush();
    decoder.Uint();
  } while (false);
}


JavaVM* gJavaVM = NULL;

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
  LOGD("%s %d", __FUNCTION__, __LINE__);
  JNIEnv* env = NULL;
  if (vm->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK) {
    return -1;
  }

  gJavaVM = vm;

  av_log_set_level(AV_LOG_TRACE);
  av_log_set_callback(ff_log_callback);
  return JNI_VERSION_1_6;
}

JNIEXPORT void JNI_OnUnload(JavaVM* vm, void* reserved) {
  LOGD("%s %d", __FUNCTION__, __LINE__);
}