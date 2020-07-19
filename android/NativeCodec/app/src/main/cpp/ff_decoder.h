//
// Created by cort xu on 2021/8/21.
//

#ifndef NATIVECODEC_FF_DECODER_H
#define NATIVECODEC_FF_DECODER_H
#include <stdint.h>
#include <stdlib.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/avutil.h>
#include <libavutil/log.h>
#include <libavutil/imgutils.h>
#include <libavutil/buffer.h>
#include <libavutil/error.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
};


class FFDecoder {
public:
  FFDecoder();
  ~FFDecoder();

public:
  bool Init(const char* file_path, bool abort_audio, bool abort_video);
  bool Progress();
  bool Seek(int64_t ts);
  bool Flush();
  void Uint();

private:
  bool Decode(AVPacket* pkt, AVCodecContext* context, AVFrame* frame);

private:
  bool                          inited_;
  AVFormatContext*              format_context_;
  AVCodecContext*               audio_context_ = NULL;
  AVCodecContext*               video_context_ = NULL;
  AVStream*                     audio_stream_ = NULL;
  AVStream*                     video_stream_ = NULL;
  AVFrame*                      audio_frame_ = NULL;
  AVFrame*                      video_frame_ = NULL;
};


#endif //NATIVECODEC_FF_DECODER_H
