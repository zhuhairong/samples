//
// Created by cort xu on 2021/8/12.
//

#include "video_decode.h"
#include "utils.h"
#include "bmp.h"

VideoDecode::VideoDecode() :
  inited_(false), fp_(nullptr), media_codec_(nullptr), media_format_(nullptr) {
}

VideoDecode::~VideoDecode() {
  Uint();
}

bool VideoDecode::Init(const std::string& file_path, const std::string& base_dir) {
  do {
    if (inited_) {
      break;
    }

    stats_.start_stamp = GetTickCount();

    base_dir_ = base_dir;

    if (!(fp_ = fopen(file_path.c_str(), "rb"))) {
      LOGE("%s %d", __FUNCTION__, __LINE__);
      break;
    }

    if (!H264Sources::GetInstance()->Get(file_path, info_)) {
      break;
    }

    stats_.frame_duration = AV_TIME_BASE / info_.fps;
    const char* mine = "video/avc";
    media_codec_ =  AMediaCodec_createDecoderByType(mine);
    media_format_ = AMediaFormat_new();
    AMediaFormat_setString(media_format_, "mime", mine);
//    AMediaFormat_setInt32(media_format_, AMEDIAFORMAT_KEY_COLOR_FORMAT, 21);
    AMediaFormat_setInt32(media_format_, AMEDIAFORMAT_KEY_WIDTH, info_.width); // 视频宽度
    AMediaFormat_setInt32(media_format_, AMEDIAFORMAT_KEY_HEIGHT, info_.height); // 视频高度

    LOGD("%s %d AMediaCodec_configure: %s", __FUNCTION__, __LINE__, AMediaFormat_toString(media_format_));

    media_status_t status = AMEDIA_OK;
    if ((status = AMediaCodec_configure(media_codec_, media_format_, NULL, NULL, 0))) {
      LOGE("%s %d status: %d", __FUNCTION__, __LINE__, status);
      break;
    }

    LOGD("%s %d AMediaCodec_configure %s", __FUNCTION__, __LINE__, AMediaFormat_toString(media_format_));

    if ((status = AMediaCodec_start(media_codec_))) {
      LOGE("%s %d status: %d", __FUNCTION__, __LINE__, status);
      break;
    }

    inited_ = true;
  } while (false);

  LOGI("%s %d inited: %s nalus: %u", __FUNCTION__, __LINE__, inited_ ? "success" : "false", info_.nalus.size());
  return inited_;
}

void VideoDecode::Progress() {
  while (true) {
    if (stats_.in_eof && stats_.ou_eof) {
      break;
    }

    if (!stats_.in_eof) {
      TryDecSend();
    }

    TryDecRecv();
  }
}

void VideoDecode::Uint() {
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

    LOGI("videodecode stats %s", stats_.toString().c_str());
}

bool VideoDecode::TryDecSend() {
  ssize_t bufidx = AMediaCodec_dequeueInputBuffer(media_codec_, MEDIACODEC_INPUT_TIMEOUT_USEC);
  LOGI("%s %d AMediaCodec_dequeueInputBuffer ret (%d)", __FUNCTION__, __LINE__, bufidx);
  if (bufidx < 0) {
    LOGE("%s %d AMediaCodec_dequeueInputBuffer fail (%d)", __FUNCTION__, __LINE__, bufidx);
    if (bufidx == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
      stats_.in_fail_again_cnt ++;
    }
    stats_.in_fail_cnt ++;
    return false;
  }

  ++ stats_.in_succ_cnt;

  if (info_.nalus.empty()) {
    stats_.in_eof = true;
    AMediaCodec_queueInputBuffer(media_codec_, bufidx, 0, 0, 0, AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM);
    LOGD("%s %d AMediaCodec_queueInputBuffer AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM", __FUNCTION__, __LINE__);
  } else {
    auto it = info_.nalus.begin();
    NaluInfo nalu = *it;
    info_.nalus.pop_front();

    size_t inbufsize = 0;
    uint8_t *inbuf = AMediaCodec_getInputBuffer(media_codec_, bufidx, &inbufsize);
    if (!inbuf) {
      return false;
    }

    uint32_t start_stamp = GetTickCount();
    int ret = fread(inbuf, 1, nalu.size, fp_);
    uint32_t end_stamp = GetTickCount();
    stats_.io_spent += (end_stamp - start_stamp);
    if (ret <= 0) {
      LOGE("%s %d fread fail (%d)", __FUNCTION__, __LINE__, ret);
    }

    AMediaCodec_queueInputBuffer(media_codec_, bufidx, 0, nalu.size, stats_.frame_pts, 0);
    stats_.frame_pts += stats_.frame_duration;
  }

  return true;
}

bool VideoDecode::TryDecRecv() {
  AMediaCodecBufferInfo info;
  ssize_t status = AMediaCodec_dequeueOutputBuffer(media_codec_, &info, MEDIACODEC_OUTPUT_TIMEOUT_USEC);
  LOGI("%s %d AMediaCodec_dequeueOutputBuffer ret (%d)", __FUNCTION__, __LINE__, status);
  if (status < 0) {
    stats_.ou_fail_cnt ++;
    if (status == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
      stats_.ou_fail_again_cnt ++;
    } else if (status == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
      stats_.ou_fail_format_cnt ++;
      AMediaFormat* format = AMediaCodec_getOutputFormat(media_codec_);
      if (format) {
        LOGD("%s %d AMediaCodec_dequeueOutputBuffer AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED format: %s",
             __FUNCTION__, __LINE__, AMediaFormat_toString(format));
        AMediaFormat_delete(format);
      }
    } else if (status == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED) {
      stats_.ou_fail_buffer_cnt ++;
    }

    return true;
  }

  stats_.ou_succ_cnt ++;
  ++ stats_.ou_frame_cnt;
  size_t oubufsize = 0;
  uint8_t* oubuf = AMediaCodec_getOutputBuffer(media_codec_, status, &oubufsize);

  LOGD("%s %d buff: (%p [%u %u %u %u %u]) oustatus: %u oubufsize: %u info: [size: %u offset: %u flags: %u pts: %lld]",
       __FUNCTION__, __LINE__, oubuf, oubuf[0], oubuf[1], oubuf[2], oubuf[3], oubuf[4], status, oubufsize,
       info.size, info.offset, info.flags, info.presentationTimeUs);

  if (info.flags & AMEDIACODEC_BUFFER_FLAG_CODEC_CONFIG) {
    stats_.ou_frame_conf_cnt ++;
  }

  if (info.flags & AMEDIACODEC_CONFIGURE_FLAG_ENCODE) {
    stats_.ou_frame_idr_cnt ++;
  }

  if (info.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM) {
    stats_.ou_eof = true;
    stats_.ou_frame_end_cnt ++;
    LOGD("%s %d AMediaCodec_dequeueOutputBuffer AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM", __FUNCTION__, __LINE__);
  }

  AMediaCodec_releaseOutputBuffer(media_codec_, status, false);//info.size != 0);
  return true;
}