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

public:
  uint32_t video_frame_sent_cnt() const { return video_frame_sent_cnt_; }
  uint32_t video_frame_recv_cnt() const { return video_frame_recv_cnt_; }
  uint32_t audio_frame_sent_cnt() const { return audio_frame_sent_cnt_; }
  uint32_t audio_frame_recv_cnt() const { return audio_frame_recv_cnt_; }

private:
  bool Decode(AVPacket* pkt, AVCodecContext* context, AVFrame* frame);

private:
  bool                          hardware_;
  bool                          multithread_;
  std::atomic_bool              inited_;
  uint32_t                      video_frame_idx_;
  uint32_t                      audio_frame_idx_;
  uint32_t                      video_frame_sent_cnt_;
  uint32_t                      video_frame_recv_cnt_;
  uint32_t                      audio_frame_sent_cnt_;
  uint32_t                      audio_frame_recv_cnt_;
  FILE*                         fp_audio_;
  AVFormatContext*              format_context_;
  AVCodecContext*               audio_context_;
  AVCodecContext*               video_context_;
  AVStream*                     audio_stream_;
  AVStream*                     video_stream_;
  AVFrame*                      audio_frame_;
  AVFrame*                      video_frame_;
};


#endif //NATIVECODEC_FF_DECODER_H
