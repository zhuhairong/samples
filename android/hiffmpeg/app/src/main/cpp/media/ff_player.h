//
// Created by cort xu on 2021/8/21.
//

#ifndef NATIVECODEC_FF_PLAYER_H
#define NATIVECODEC_FF_PLAYER_H
#include <stdint.h>
#include <stdlib.h>
#include <jni.h>
#include <string>
#include <future>
#include <unistd.h>
#include <android/native_window_jni.h>
#include "utils.h"

struct FFPlayerInfo {
    bool      audio_enable = false;

    bool      video_enable = false;
    uint32_t  surface_width = 0;
    uint32_t  surface_height = 0;
};

class FFPlayer {
public:
  FFPlayer(bool hardware);
  ~FFPlayer();

public:
  bool Init(JNIEnv *env, jstring input, jstring  output_path, jobject surface, const FFPlayerInfo& info);
  void Process();
  void Uint();

private:
  bool Seek(int64_t ts);
  bool Flush();
  bool Decode(AVPacket* pkt, AVCodecContext* context, AVFrame* frame);

private:
  bool                          hardware_;
  std::atomic_bool              inited_;
  std::string                   output_path_;
  uint32_t                      video_frame_idx_;
  uint32_t                      audio_frame_idx_;
  ANativeWindow*                native_window_;
  AVFormatContext*              format_context_;
  AVCodecContext*               audio_context_;
  AVCodecContext*               video_context_;
  AVStream*                     audio_stream_;
  AVStream*                     video_stream_;
  AVFrame*                      audio_frame_;
  AVFrame*                      video_frame_;
  AVFrame*                      rgb_frame_;
  SwsContext*                   sws_ctx_;
};


#endif //NATIVECODEC_FF_PLAYER_H
