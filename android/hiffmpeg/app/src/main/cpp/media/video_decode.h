//
// Created by cort xu on 2021/8/12.
//

#ifndef NATIVECODEC_VIDEO_DECODE_H
#define NATIVECODEC_VIDEO_DECODE_H
#include "media_codec.h"
#include "h264_nalu_parse.h"
#include "h264_sources.h"

class VideoDecode {
public:
  VideoDecode();
  ~VideoDecode();

public:
  bool Init(const std::string& file_path, const std::string& base_dir);
  void Progress();
  void Uint();

private:
    bool TryDecSend();
    bool TryDecRecv();

private:
  bool                inited_;
  std::string         base_dir_;
  VideoStats          stats_;
  H264Info            info_;
  FILE*               fp_;
  AMediaCodec*        media_codec_;
  AMediaFormat*       media_format_;
};


#endif //NATIVECODEC_VIDEO_DECODE_H
