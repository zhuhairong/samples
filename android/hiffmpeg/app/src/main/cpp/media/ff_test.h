//
// Created by cort xu on 2021/8/21.
//

#ifndef NATIVECODEC_FF_TEST_H
#define NATIVECODEC_FF_TEST_H
#include <stdint.h>
#include <stdlib.h>
#include <jni.h>
#include <string>
#include <future>
#include <unistd.h>
#include <android/native_window_jni.h>

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


class FFTest {
public:
  FFTest();
  ~FFTest();

public:
  bool Init(JNIEnv *env, jstring path, jobject surface, bool abort_audio, bool abort_video);
  bool Flush();
  void Uint();

private:
    bool Decode(AVPacket* pkt, AVCodecContext* context, AVFrame* frame);

private:
  std::atomic_bool              inited_;
  ANativeWindow_Buffer          native_buffer_;
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


#endif //NATIVECODEC_FF_TEST_H
