//
// Created by cort xu on 2021/8/21.
//

#ifndef NATIVECODEC_FF_DECODER_H
#define NATIVECODEC_FF_DECODER_H
#include <stdint.h>
#include <stdlib.h>
#include <jni.h>
#include <string>
#include <future>
#include <unistd.h>
#include <android/native_window_jni.h>
#include "utils.h"

struct FFDecoderInfo {
    std::string   file_path;
    std::string   output_dir;
    bool          enable_video;
    bool          enable_audio;
};

struct FFDecoderStats {
    uint32_t start_stamp = 0;
    uint32_t end_stamp = 0;
    uint32_t audio_sent_cnt = 0;
    uint32_t video_sent_cnt = 0;
    uint32_t audio_recv_cnt = 0;
    uint32_t video_recv_cnt = 0;
};

class FFDecoder {
public:
  FFDecoder(bool hardware, bool multithread);
  ~FFDecoder();

public:
  bool Init(const FFDecoderInfo& info);
  bool Process();
  bool Seek(int64_t ts);
  bool Flush();
  void Uint();

private:
  bool Decode(AVPacket* pkt, AVCodecContext* context, AVFrame* frame);

private:
  bool                          hardware_;
  bool                          multithread_;
  std::atomic_bool              inited_;
  uint32_t                      video_frame_idx_;
  uint32_t                      audio_frame_idx_;
  FILE*                         fp_audio_;
  FFDecoderStats                stats_;
  AVFormatContext*              format_context_;
  AVCodecContext*               audio_context_;
  AVCodecContext*               video_context_;
  AVStream*                     audio_stream_;
  AVStream*                     video_stream_;
  AVFrame*                      audio_frame_;
  AVFrame*                      video_frame_;
};


#endif //NATIVECODEC_FF_DECODER_H
