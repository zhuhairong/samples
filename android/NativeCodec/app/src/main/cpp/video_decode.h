//
// Created by cort xu on 2021/8/12.
//

#ifndef NATIVECODEC_VIDEO_DECODE_H
#define NATIVECODEC_VIDEO_DECODE_H
#include "media_codec.h"
#include "h264_nalu_parse.h"

class MediaDecode : public MediaCodec {
public:
  MediaDecode();
  ~MediaDecode();

public:
  bool Init(const std::string& file_path);
  bool Progress();
  bool Flush();
  void Uint();

private:
    bool Decode(uint8_t* data, uint32_t size);

private:
  bool                inited_;
  std::list<NaluUnit> nalus_;
  H264NaluParse       nalu_parse_;
  FILE*               fp_;
  AMediaCodec*        media_codec_;
  AMediaFormat*       media_format_;
};


#endif //NATIVECODEC_VIDEO_DECODE_H
