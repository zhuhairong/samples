//
// Created by cort xu on 2021/8/12.
//

#ifndef NATIVECODEC_VIDEO_ENCODE_H
#define NATIVECODEC_VIDEO_ENCODE_H
#include "media_codec.h"

class VideoEncode : public MediaCodec {
public:
  VideoEncode();
  ~VideoEncode();

public:
  bool Init(int width, int height, int fps, int bitrate, const std::string& file_path);
  bool Encode(void* data, int size, int64_t pts);
  bool Flush();
  void Uint();

private:
  bool EncodeFrame(void* data, int size, int64_t pts);
  bool RecvFrame();

private:
  bool          inited_;
  uint32_t      frame_idx_;
  FILE*         fp_;
  AMediaCodec*  media_codec_;
  AMediaFormat* media_format_;
};



#endif //NATIVECODEC_VIDEO_ENCODE_H
