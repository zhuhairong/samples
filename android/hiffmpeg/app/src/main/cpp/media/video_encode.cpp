//
// Created by cort xu on 2021/8/12.
//

#include "video_encode.h"
#include "utils.h"

VideoEncode::VideoEncode() : inited_(false), media_codec_(nullptr), media_format_(nullptr) {
}

VideoEncode::~VideoEncode() {
  Uint();
}

bool VideoEncode::Init(const VideoEncodeParmas& parmas) {
  do {
    if (inited_) {
      break;
    }

    parmas_ = parmas;
    stats_.start_stamp = GetTickCount();
    stats_.frame_duration = AV_TIME_BASE / parmas.fps;
    stats_.frame_size = parmas.width * parmas.height * 3 / 2;

    if (!(fp_ = fopen(parmas_.file_path.c_str(), "wb"))) {
      LOGE("%s fopen: %s fail", __FUNCTION__, parmas_.file_path.c_str());
      break;
    }

    const char* mine = "video/avc";
    media_codec_ =  AMediaCodec_createEncoderByType(mine);
    media_format_ = AMediaFormat_new();
    AMediaFormat_setString(media_format_, "mime", mine);

    AMediaFormat_setInt32(media_format_, AMEDIAFORMAT_KEY_WIDTH, parmas_.width); // 视频宽度
    AMediaFormat_setInt32(media_format_, AMEDIAFORMAT_KEY_HEIGHT, parmas_.height); // 视频高度
    AMediaFormat_setInt32(media_format_, AMEDIAFORMAT_KEY_BIT_RATE, parmas_.bitrate);
    AMediaFormat_setInt32(media_format_, AMEDIAFORMAT_KEY_FRAME_RATE, parmas_.fps);
    AMediaFormat_setInt32(media_format_, AMEDIAFORMAT_KEY_I_FRAME_INTERVAL, 1);//s
    AMediaFormat_setInt32(media_format_, AMEDIAFORMAT_KEY_COLOR_FORMAT, 21);
    AMediaFormat_setInt32(media_format_, "bitrate-mode", MEDIACODEC_BITRATE_MODE_VBR);
    media_status_t status = AMEDIA_OK;
    if ((status = AMediaCodec_configure(media_codec_, media_format_, NULL, NULL, AMEDIACODEC_CONFIGURE_FLAG_ENCODE))) {
      LOGD("%s %d AMediaCodec_configure fail (%d)", __FUNCTION__, __LINE__, status);
      break;
    }

    LOGD("%s %d AMediaCodec_configure status: %d %s", __FUNCTION__, __LINE__, status, AMediaFormat_toString(media_format_));

    if ((status = AMediaCodec_start(media_codec_))) {
      LOGD("%s %d AMediaCodec_start fail (%d)", __FUNCTION__, __LINE__, status);
      break;
    }

    inited_ = true;
  } while (false);

  LOGI("%s %d inited: %s", __FUNCTION__, __LINE__, inited_ ? "success" : "false");
  return inited_;
}

void VideoEncode::Progress() {
  while (true) {
    if (stats_.in_eof && stats_.ou_eof) {
        break;
    }

    if (!stats_.in_eof) {
      TryEncSend();
    }

    TryEncRecv();
  }
}

void VideoEncode::Uint() {
  if (fp_) {
    fclose(fp_);
    fp_ = nullptr;
  }

  if (media_format_) {
    AMediaFormat_delete(media_format_);
    media_format_ = nullptr;
  }

  if (media_codec_) {
    AMediaCodec_stop(media_codec_);
    AMediaCodec_delete(media_codec_);
    media_codec_ = nullptr;
  }

  if (!inited_) {
    return;
  }

  stats_.end_stamp = GetTickCount();
  inited_ = false;

  LOGI("videoencode stats %s", stats_.toString().c_str());
}

bool VideoEncode::TryEncSend() {
  do {
    int bufidx = AMediaCodec_dequeueInputBuffer(media_codec_, MEDIACODEC_INPUT_TIMEOUT_USEC);
    LOGI("%s %d AMediaCodec_dequeueInputBuffer ret (%d)", __FUNCTION__, __LINE__, bufidx);
    if (bufidx < 0) {
      LOGE("%s %d AMediaCodec_dequeueInputBuffer fail (%d)", __FUNCTION__, __LINE__, bufidx);
      stats_.in_fail_cnt ++;
      break;
    }

    ++ stats_.in_succ_cnt;
    if (stats_.in_succ_cnt >= parmas_.frame_cnt) {
      stats_.in_eof = true;

      AMediaCodec_queueInputBuffer(media_codec_, bufidx, 0, 0, stats_.frame_pts,
                                   AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM);
      LOGD("%s %d AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM", __FUNCTION__, __LINE__);
    } else {
      size_t bufsize = 0;
      uint8_t *buf = AMediaCodec_getInputBuffer(media_codec_, bufidx, &bufsize);
      if (!buf) {
        LOGE("%s %d AMediaCodec_dequeueInputBuffer fail", __FUNCTION__, __LINE__);
        break;
      }

      uint32_t start_stamp = GetTickCount();
      FillRandData(buf, bufsize);
      uint32_t end_stamp = GetTickCount();
      stats_.fill_spent += (end_stamp - start_stamp);

      media_status_t status = AMediaCodec_queueInputBuffer(media_codec_, bufidx, 0, stats_.frame_size, stats_.frame_pts, 0);
      if (status != AMEDIA_OK) {
        LOGE("%s %d AMediaCodec_queueInputBuffer fail (%d)", __FUNCTION__, __LINE__, status);
      }
    }

    stats_.frame_pts += stats_.frame_duration;
  } while (false);
  return true;
}

bool VideoEncode::TryEncRecv() {
  while (true) {
    AMediaCodecBufferInfo info;
    size_t bufsize = 0;
    ssize_t status = AMediaCodec_dequeueOutputBuffer(media_codec_, &info, MEDIACODEC_OUTPUT_TIMEOUT_USEC);
    LOGI("%s %d AMediaCodec_dequeueOutputBuffer ret (%d)", __FUNCTION__, __LINE__, status);
    if (status < 0) {
      stats_.ou_fail_cnt ++;
      if (status == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
        stats_.ou_fail_again_cnt ++;
        break;
      } else if (status == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
        stats_.ou_fail_format_cnt ++;

        AMediaFormat *format = AMediaCodec_getOutputFormat(media_codec_);
        if (format) {
          uint8_t *sps = nullptr;
          size_t sps_len = 0;
          uint8_t *pps = nullptr;
          size_t pps_len = 0;
          bool sps_ok = AMediaFormat_getBuffer(format, "csd-0", (void **) &sps, &sps_len);
          bool pps_ok = AMediaFormat_getBuffer(format, "csd-1", (void **) &pps, &pps_len);
          if (sps_ok && pps_ok) {
            int sps_type = sps[4] & 0x1f;
            int pps_type = pps[4] & 0x1f;
            LOGD("%s %d AMediaCodec_dequeueOutputBuffer AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED sps_type: %d sps_len: %u pps_type: %d pps_len: %u",
                 __FUNCTION__, __LINE__, sps_type, sps_len, pps_type, pps_len);
          }
          LOGD("%s %d AMediaCodec_dequeueOutputBuffer AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED %s",
               __FUNCTION__, __LINE__, AMediaFormat_toString(format));

          AMediaFormat_delete(format);
        }
        continue;
      } else if (status == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED) {
        stats_.ou_fail_buffer_cnt ++;
        LOGD("%s %d AMediaCodec_dequeueOutputBuffer AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED",
             __FUNCTION__, __LINE__);
        continue;
      } else {
        break;
      }
    }

    stats_.ou_succ_cnt ++;

    stats_.ou_frame_cnt ++;

    uint8_t *buf = AMediaCodec_getOutputBuffer(media_codec_, status, &bufsize);
//    LOGD("%s %d buf: %p status: %u bufsize: %u", __FUNCTION__, __LINE__, buf, status, bufsize);
    int type = buf[4] & 0x1f;

    LOGD("nalu, AMediaCodec_dequeueOutputBuffer type: %d bufsize: %d info: (size: %u flags: %u offset: %u pts: %lld) data: [%u %u %u %u %u]",
         type, bufsize, info.size, info.flags, info.offset, info.presentationTimeUs, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);

    if (info.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM) {
      stats_.ou_eof = true;
      stats_.ou_frame_end_cnt ++;
    }

    if (info.flags & AMEDIACODEC_BUFFER_FLAG_CODEC_CONFIG) {
      stats_.ou_frame_conf_cnt ++;
    }

    if (info.flags & AMEDIACODEC_CONFIGURE_FLAG_ENCODE) {
      stats_.ou_frame_idr_cnt ++;
    }

    if (fp_) {
      uint32_t start_stamp = GetTickCount();
      fwrite(buf, 1, info.size, fp_);
      uint32_t end_stamp = GetTickCount();
      stats_.io_spent += (end_stamp - start_stamp);
    }

    AMediaCodec_releaseOutputBuffer(media_codec_, status, false);
  }

  return true;
}