//
// Created by cort xu on 2021/8/12.
//


#ifndef NATIVECODEC_MEDIA_CODEC_H
#define NATIVECODEC_MEDIA_CODEC_H
#include <media/NdkMediaCodec.h>
#include <media/NdkMediaFormat.h>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <sstream>

#define MEDIACODEC_BITRATE_MODE_CQ  0 //MediaCodecInfo.EncoderCapabilities.BITRATE_MODE_CQ
#define MEDIACODEC_BITRATE_MODE_VBR 1 //MediaCodecInfo.EncoderCapabilities.BITRATE_MODE_VBR
#define MEDIACODEC_BITRATE_MODE_CBR 2 //MediaCodecInfo.EncoderCapabilities.BITRATE_MODE_CBR
#define MEDIACODEC_INPUT_TIMEOUT_USEC 100000//us
#define MEDIACODEC_OUTPUT_TIMEOUT_USEC 16000//us

typedef struct {
  bool      in_eof = false;
  bool      ou_eof = false;

  uint32_t  frame_duration = 0;
  uint32_t  frame_pts = 0;
  uint32_t  frame_size = 0;

  uint32_t  start_stamp = 0;
  uint32_t  end_stamp = 0;
  uint32_t  fill_spent = 0;
  uint32_t  io_spent = 0;

  uint32_t  in_succ_cnt = 0;
  uint32_t  in_fail_cnt = 0;
  uint32_t  in_fail_again_cnt = 0;

  uint32_t  ou_succ_cnt = 0;
  uint32_t  ou_fail_cnt = 0;
  uint32_t  ou_fail_again_cnt = 0;
  uint32_t  ou_fail_format_cnt = 0;
  uint32_t  ou_fail_buffer_cnt = 0;

  uint32_t  ou_frame_cnt = 0;
  uint32_t  ou_frame_conf_cnt = 0;
  uint32_t  ou_frame_idr_cnt = 0;
  uint32_t  ou_frame_end_cnt = 0;

  std::string toString() const {
    std::ostringstream oss;

    do {
      if (!ou_frame_cnt) {
        break;
      }

      uint32_t all_spent = end_stamp - start_stamp;
      uint32_t codec_spent = all_spent - fill_spent - io_spent;
      oss << "spent: (all: " << all_spent << " fill: " << fill_spent << " io: " << io_spent << " codec: " << codec_spent << ") avg: " << (codec_spent / ou_frame_cnt);
      oss << " in: (succ: " << in_succ_cnt << " fail: " << in_fail_cnt << " again: " << in_fail_again_cnt << ")";
      oss << " ou: (succ: " << ou_succ_cnt << " fail: " << ou_fail_cnt << " again: " << ou_fail_again_cnt << " format: " << ou_fail_format_cnt << " buffer: " << ou_fail_buffer_cnt << ")";
      oss << " frame: (all: " << ou_frame_cnt << " conf: " << ou_frame_conf_cnt << " idr: " << ou_frame_idr_cnt << " end: " << ou_frame_end_cnt << ")";
    } while (false);

    return oss.str();
  }
} VideoStats;

#endif //NATIVECODEC_MEDIA_CODEC_H
