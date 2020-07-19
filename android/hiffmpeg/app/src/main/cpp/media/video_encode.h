//
// Created by cort xu on 2021/8/12.
//

#ifndef NATIVECODEC_VIDEO_ENCODE_H
#define NATIVECODEC_VIDEO_ENCODE_H
#include "media_codec.h"

typedef struct {
  int         frame_cnt = 1000;
  int         width = 0;
  int         height = 0;
  int         fps = 0;
  int         bitrate = 0;
  std::string file_path;
} VideoEncodeParmas;

class VideoEncode {
public:
  VideoEncode();
  ~VideoEncode();

public:
  bool Init(const VideoEncodeParmas& parmas);
  void Progress();
  void Uint();

private:
  bool TryEncSend();
  bool TryEncRecv();

private:
  bool                inited_;
  VideoEncodeParmas   parmas_;
  VideoStats          stats_;
  FILE*               fp_;
  AMediaCodec*        media_codec_;
  AMediaFormat*       media_format_;
};



#endif //NATIVECODEC_VIDEO_ENCODE_H
