//
// Created by cort xu on 2021/9/6.
//

#ifndef HIFFMPEG_FF_ENCODER_H
#define HIFFMPEG_FF_ENCODER_H
#include <stdint.h>
#include <stdlib.h>
#include <jni.h>
#include <string>
#include <future>
#include <unistd.h>
#include <android/native_window_jni.h>
#include "utils.h"

struct FFEncoderInfo {
    std::string       file_path;

    bool              video_enable = false;
    AVPixelFormat     video_format;
    int               video_width;
    int               video_height;
    int               video_fps;
    int               video_gop;
    int               video_bitrate;

    bool              audio_enable = false;
    AVSampleFormat    audio_format;
    int               audio_channel;
    int               audio_samplerate;
    int               audio_bitrate;
};

class FFEncoder {
public:
  FFEncoder();
  ~FFEncoder();

public:
  bool Init(const FFEncoderInfo& info);
  bool EncodeVideo(uint8_t* data, uint32_t size, int64_t pts);
  bool EncodeAudio(uint8_t* data, uint32_t size, int64_t pts);
  void Uint();

private:
    bool Encode(AVCodecContext* context, AVFrame* frame);

private:
    bool                          inited_ = false;
    uint32_t                      audio_frame_duration_ = 0;
    uint32_t                      video_frame_duration_ = 0;
    AVFormatContext*              format_context_ = NULL;
    AVCodecContext*               audio_context_ = NULL;
    AVCodecContext*               video_context_ = NULL;
    AVStream*                     audio_stream_ = NULL;
    AVStream*                     video_stream_ = NULL;
    AVFrame*                      audio_frame_ = NULL;
    AVFrame*                      video_frame_ = NULL;
};


#endif //HIFFMPEG_FF_ENCODER_H
