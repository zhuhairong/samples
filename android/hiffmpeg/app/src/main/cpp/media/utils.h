//
// Created by cort xu on 2021/8/12.
//

#ifndef NATIVECODEC_UTILS_H
#define NATIVECODEC_UTILS_H
#include <android/log.h>
#include <string.h>
#include <string>
#include <time.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

extern "C" {
#include "../../../../libs/ffmpeg/include/libavformat/avformat.h"
#include "../../../../libs/ffmpeg/include/libavcodec/avcodec.h"
#include "../../../../libs/ffmpeg/include/libavcodec/jni.h"
#include "../../../../libs/ffmpeg/include/libavutil/time.h"
#include "../../../../libs/ffmpeg/include/libavutil/opt.h"
#include "../../../../libs/ffmpeg/include/libavutil/audio_fifo.h"
#include "../../../../libs/ffmpeg/include/libavutil/avutil.h"
#include "../../../../libs/ffmpeg/include/libavutil/log.h"
#include "../../../../libs/ffmpeg/include/libavutil/imgutils.h"
#include "../../../../libs/ffmpeg/include/libavutil/buffer.h"
#include "../../../../libs/ffmpeg/include/libavutil/error.h"
#include "../../../../libs/ffmpeg/include/libswresample/swresample.h"
#include "../../../../libs/ffmpeg/include/libswscale/swscale.h"
#include "../../../../libs/ffmpeg/include/libavfilter/avfilter.h"
#include "../../../../libs/ffmpeg/include/libavfilter/buffersrc.h"
#include "../../../../libs/ffmpeg/include/libavfilter/buffersink.h"
};


#define SAFE_DELETE(x) { if(x){ delete x; x = NULL;}}
#define SAFE_DELETE_ARRAY(x) { if(x){ delete[] x; x = NULL;}}

#define TAG "[hilive]"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

struct HwEncodeParameter {
  const char* mime;
  int         profile = 0;
  int         bitrate = 0;
  int         width = 0;
  int         height = 0;
  int         fps = 0;
  int         gop = 0;
  int         channel = 0;
  int         samplerate = 0;
};

inline uint32_t GetTickCount() {
  struct timespec on = {0, 0};
  if (clock_gettime(CLOCK_MONOTONIC, &on) != 0) {
    return 0;
  }

  unsigned int tick = on.tv_sec * 1000 + on.tv_nsec / 1000000;
  return tick;
}

inline bool CreatePath(const std::string& path) {
  if (path.empty()) {
    return false;
  }

  std::string full_path = path;

  size_t pos = 0;
  std::string sub_path;
  while (sub_path != full_path) {
    pos = full_path.find_first_of("/", pos + 1);
    if (pos == std::string::npos) {
      sub_path = full_path;
    } else {
      sub_path = full_path.substr(0, pos);
    }

    if (access(sub_path.c_str(), F_OK) == 0) {
      continue;
    }

    if (mkdir(sub_path.c_str(), 0755) != 0) {
      return false;
    }
  }

  return true;
}

inline bool CreateParentPath(const std::string& file_path) {
  size_t pos = file_path.find_last_of("/");
  if (pos == std::string::npos) {
    return false;
  }

  return CreatePath(file_path.substr(0, pos));
}


inline AVCodec* FFGetDecoder(int codec_id, bool hardware) {
  LOGD("%s hardware: %s avcodec_find_decoder: %d", __FUNCTION__, hardware ? "yes" : "no", codec_id);

  switch (codec_id) {
    case AV_CODEC_ID_H264: {
      return avcodec_find_decoder_by_name(hardware ? "h264_hlmediacodec" : "h264");
    }
    case AV_CODEC_ID_HEVC:
      return avcodec_find_decoder_by_name(hardware ? "hevc_hlmediacodec" : "hevc");
    case AV_CODEC_ID_AAC:
      return avcodec_find_decoder_by_name(hardware ? "aac_hlmediacodec" : "aac");
    case AV_CODEC_ID_MP3:
      return avcodec_find_decoder_by_name(hardware ? "mp3_hlmediacodec" : "mp3");
    default: {
      return avcodec_find_decoder((AVCodecID) codec_id);
    }
  }
}

inline AVCodec* FFGetEncoder(int codec_id, bool hardware) {
  if (!hardware) {
    return avcodec_find_encoder((AVCodecID)codec_id);
  }

  switch (codec_id) {
    case AV_CODEC_ID_H264:
      return avcodec_find_encoder_by_name("h264_hlmediacodec");
    case AV_CODEC_ID_HEVC:
      return avcodec_find_encoder_by_name("hevc_hlmediacodec");
    default:
      return avcodec_find_decoder((AVCodecID)codec_id);
  }
}

//media ffmpeg stamp to s
inline double ff_stamp2s(double size, AVRational a) {
  double num = a.num;
  double den = a.den;
  if (num == 0 || den == 0)
  {
    return  0;
  }

  return size * num / den;
}

//media ffmpeg stamp to ms
inline double ff_stamp2ms(double size, AVRational a) {
  return 1000 * ff_stamp2s(size, a);
}

//media ffmpeg stamp to us
inline double ff_stamp2us(double size, AVRational a) {
  return AV_TIME_BASE * ff_stamp2s(size, a);
}

inline double ff_stamp2us_q(double stamp, AVRational a) {
  double duration = ff_stamp2us(1, a);
  if (duration <= 0)
  {
    return 0;
  }

  return stamp / duration;
}


inline void FillRandData(void* data, uint32_t size) {
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


#endif //NATIVECODEC_UTILS_H
