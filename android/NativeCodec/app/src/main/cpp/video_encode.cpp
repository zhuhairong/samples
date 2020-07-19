//
// Created by cort xu on 2021/8/12.
//

#include "video_encode.h"
#include "utils.h"

VideoEncode::VideoEncode() : inited_(false), frame_idx_(0), media_codec_(nullptr), media_format_(nullptr) {
}

VideoEncode::~VideoEncode() {
  Uint();
}

bool VideoEncode::Init(int width, int height, int fps, int bitrate, const std::string& file_path) {
  do {
    if (inited_) {
      break;
    }

    if (!(fp_ = fopen(file_path.c_str(), "wb"))) {
      break;
    }

    const char* mine = "video/avc";
    media_codec_ =  AMediaCodec_createEncoderByType(mine);
    media_format_ = AMediaFormat_new();
    AMediaFormat_setString(media_format_, "mime", mine);

    AMediaFormat_setInt32(media_format_, AMEDIAFORMAT_KEY_WIDTH, width); // 视频宽度
    AMediaFormat_setInt32(media_format_, AMEDIAFORMAT_KEY_HEIGHT, height); // 视频高度

    AMediaFormat_setInt32(media_format_, AMEDIAFORMAT_KEY_BIT_RATE, bitrate);
    AMediaFormat_setInt32(media_format_, AMEDIAFORMAT_KEY_FRAME_RATE, fps);
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

bool VideoEncode::Encode(void* data, int size, int64_t pts) {
  if (!inited_) {
    return false;
  }

  if (!EncodeFrame(data, size, pts)) {
    return false;
  }

  if (!RecvFrame()) {
    return false;
  }

  return true;
}

bool VideoEncode::Flush() {
  if (!inited_) {
    return false;
  }

  if (!EncodeFrame(nullptr, 0, 0)) {
    return false;
  }

  if (!RecvFrame()) {
    return false;
  }

  return true;
}

bool VideoEncode::EncodeFrame(void* data, int size, int64_t pts) {
  int bufidx = AMediaCodec_dequeueInputBuffer(media_codec_, MEDIACODEC_TIMEOUT_USEC);
  if (bufidx < 0) {
    LOGE("%s %d AMediaCodec_dequeueInputBuffer fail (%d)", __FUNCTION__, __LINE__, bufidx);
    return false;
  }

  if (!data) {
    AMediaCodec_queueInputBuffer(media_codec_, bufidx, 0, 0, pts, AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM);
    LOGD("%s %d AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM: %d", __FUNCTION__, __LINE__, AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM);
    return true;
  }

  size_t bufsize = 0;
  uint8_t* buf = AMediaCodec_getInputBuffer(media_codec_, bufidx, &bufsize);
  if (!buf) {
    LOGE("%s %d AMediaCodec_dequeueInputBuffer fail", __FUNCTION__, __LINE__);
    return false;
  }

  LOGD("%s %d AMediaCodec_queueInputBuffer bfidx: %d bfsize: %u size: %d pts: %lld", __FUNCTION__, __LINE__, bufidx, bufsize, size, pts);
  memcpy(buf, data, size);

  ++ frame_idx_;
  uint32_t flags = 0;
  if (frame_idx_ % 5 == 0) {
    flags |= AMEDIACODEC_CONFIGURE_FLAG_ENCODE;
  } else if (frame_idx_ % 10 == 0) {
    flags |= AMEDIACODEC_BUFFER_FLAG_CODEC_CONFIG;
  }

  media_status_t status = AMediaCodec_queueInputBuffer(media_codec_, bufidx, 0, size, pts, flags);
//  LOGD("%s %d AMediaCodec_queueInputBuffer status (%d)", __FUNCTION__, __LINE__, status);
  return true;
}

bool VideoEncode::RecvFrame() {
  while (true) {
    AMediaCodecBufferInfo info;
    size_t bufsize = 0;
    ssize_t status = AMediaCodec_dequeueOutputBuffer(media_codec_, &info, MEDIACODEC_TIMEOUT_USEC);
    if (status < 0) {
      if (status == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
//        LOGD("%s %d AMediaCodec_dequeueOutputBuffer AMEDIACODEC_INFO_TRY_AGAIN_LATER", __FUNCTION__, __LINE__);
        return true;
      }

      LOGD("%s %d AMediaCodec_dequeueOutputBuffer: %d", __FUNCTION__, __LINE__, status);
      if (status == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
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
        LOGD("%s %d AMediaCodec_dequeueOutputBuffer AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED",
             __FUNCTION__, __LINE__);
        continue;
      }

      return true;
    }

    uint8_t *buf = AMediaCodec_getOutputBuffer(media_codec_, status, &bufsize);
//    LOGD("%s %d buf: %p status: %u bufsize: %u", __FUNCTION__, __LINE__, buf, status, bufsize);
    int type = buf[4] & 0x1f;

    LOGD("nalu, AMediaCodec_dequeueOutputBuffer type: %d size: %u flags: %u offset: %u pts: %lld",
         type, info.size, info.flags, info.offset, info.presentationTimeUs);

    if (fp_) {
      fwrite(buf, 1, info.size, fp_);
      fflush(fp_);
    }

    AMediaCodec_releaseOutputBuffer(media_codec_, status, false);
  }
  return true;
}

void VideoEncode::Uint() {
  inited_ = false;

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
}