//
// Created by cort xu on 2021/8/12.
//

#include "video_decode.h"
#include "utils.h"

MediaDecode::MediaDecode() : inited_(false), fp_(nullptr), media_codec_(nullptr), media_format_(nullptr) {
}

MediaDecode::~MediaDecode() {
  Uint();
}

bool MediaDecode::Init(const std::string& file_path) {
  do {
    if (inited_) {
      break;
    }

    if (!(fp_ = fopen(file_path.c_str(), "rb"))) {
      LOGE("%s %d", __FUNCTION__, __LINE__);
      break;
    }

    int width = 0;
    int height = 0;
    int fps = 0;

    while (true) {
      if (!nalu_parse_.parse(nalus_, fp_)) {
        LOGE("%s %d", __FUNCTION__, __LINE__);
        break;
      }

      for (auto& nalu : nalus_) {
        if (nalu.nal_unit_type == kNaluTypeSPS) {
          uint8_t* data = (uint8_t*)(nalu.data.c_str() + nalu.mark_size);
          uint32_t len = nalu.data.size() - nalu.mark_size;
          nalu_parse_.sps(data, len, width, height, fps);
        }
      }

      if (nalu_parse_.foundPps() && nalu_parse_.foundSps()) {
        LOGD("%s %d found config", __FUNCTION__, __LINE__);
        break;
      }
    }

    const char* mine = "video/avc";
    media_codec_ =  AMediaCodec_createDecoderByType(mine);
    media_format_ = AMediaFormat_new();
    AMediaFormat_setString(media_format_, "mime", mine);
//    AMediaFormat_setInt32(media_format_, AMEDIAFORMAT_KEY_COLOR_FORMAT, 21);
    AMediaFormat_setInt32(media_format_, AMEDIAFORMAT_KEY_WIDTH, width); // 视频宽度
    AMediaFormat_setInt32(media_format_, AMEDIAFORMAT_KEY_HEIGHT, height); // 视频高度

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

  LOGI("%s %d inited: %s", __FUNCTION__, __LINE__, inited_ ? "success" : "false");
  return inited_;
}

bool MediaDecode::Progress() {
  if (!inited_) {
    return false;
  }

  if (!nalu_parse_.parse(nalus_, fp_)) {
    return false;
  }

  for (auto& nalu : nalus_) {
    LOGD("nalu, type: %d size: %u data: [%u %u %u %u %u]",
         nalu.nal_unit_type, nalu.data.size(), nalu.data[0], nalu.data[1], nalu.data[2], nalu.data[3], nalu.data[4]);
    if (!Decode((uint8_t*)nalu.data.c_str(), nalu.data.size())) {
      return false;
    }
  }

  nalus_.clear();

  return true;
}

bool MediaDecode::Flush() {
  if (!inited_) {
    return false;
  }

  return Decode(NULL, 0);
}

void MediaDecode::Uint() {
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

    nalu_parse_.reset();
}

bool MediaDecode::Decode(uint8_t* data, uint32_t size) {
  ssize_t bufidx = AMediaCodec_dequeueInputBuffer(media_codec_, MEDIACODEC_TIMEOUT_USEC);
  if (bufidx < 0) {
    LOGE("%s %d AMediaCodec_dequeueInputBuffer fail (%d)", __FUNCTION__, __LINE__, bufidx);
    return false;
  }

  int64_t pts = 0;
  size_t inbufsize = 0;
  if (data) {
    uint8_t *inbuf = AMediaCodec_getInputBuffer(media_codec_, bufidx, &inbufsize);
    memcpy(inbuf, data, size);
    AMediaCodec_queueInputBuffer(media_codec_, bufidx, 0, size, pts, 0);
  } else {
    AMediaCodec_queueInputBuffer(media_codec_, bufidx, 0, 0, pts, AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM);
    LOGD("%s %d AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM: %d", __FUNCTION__, __LINE__, AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM);
  }

  while (true) {
    AMediaCodecBufferInfo info;
    ssize_t status = AMediaCodec_dequeueOutputBuffer(media_codec_, &info, MEDIACODEC_TIMEOUT_USEC);
//    LOGD("%s %d AMediaCodec_dequeueOutputBuffer: %d", __FUNCTION__, __LINE__, status);

    if (status < 0) {
      break;
    }

    size_t oubufsize = 0;
    uint8_t* oubuf = AMediaCodec_getOutputBuffer(media_codec_, status, &oubufsize);

    LOGD("%s %d inbufsize: %u inbufidx: %u oustatus: %u oubufsize: %u info: [size: %u offset: %u flags: %u pts: %lld]",
         __FUNCTION__, __LINE__, inbufsize, bufidx, status, oubufsize, info.size, info.offset, info.flags, info.presentationTimeUs);

//    LOGD("%s %d decode data: [%u %u %u %u %u %u]", __FUNCTION__, __LINE__, oubuf[0], oubuf[10], oubuf[100], oubuf[200], oubuf[300], oubuf[600]);

    AMediaCodec_releaseOutputBuffer(media_codec_, status, false);//info.size != 0);
  }

  return true;
}