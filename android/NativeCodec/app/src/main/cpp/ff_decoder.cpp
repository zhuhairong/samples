//
// Created by cort xu on 2021/8/21.
//

#include "ff_decoder.h"
#include "utils.h"

static AVCodec* FFGetDecoder(int codec_id) {
  switch (codec_id) {
    case AV_CODEC_ID_H264:
      return avcodec_find_decoder_by_name("h264_hlmediacodec");
    case AV_CODEC_ID_HEVC:
      return avcodec_find_decoder_by_name("hevc_hlmediacodec");
    default:
      return avcodec_find_decoder((AVCodecID)codec_id);
  }
}

FFDecoder::FFDecoder()
  : inited_(false), format_context_(nullptr),
    audio_context_(nullptr), video_context_(nullptr),
    audio_stream_(nullptr), video_stream_(nullptr),
    audio_frame_(nullptr), video_frame_(nullptr) {

}

FFDecoder::~FFDecoder() {
  Uint();
}

bool FFDecoder::Init(const char* file_path, bool abort_audio, bool abort_video) {
  do {
    if (inited_) {
      break;
    }

    if (abort_audio && abort_video) {
      break;
    }

    int ret = avformat_open_input(&format_context_, file_path, nullptr, nullptr);
    if (ret < 0) {
      LOGE("%s %d avformat_open_input fail (%d %s)", __FUNCTION__, __LINE__, ret, av_err2str(ret));
      break;
    }

    if ((ret = avformat_find_stream_info(format_context_, NULL)) < 0) {
      LOGE("%s %d avformat_find_stream_info fail (%d)", __FUNCTION__, __LINE__, ret);
      break;
    }

    if (!abort_audio) {
      if ((ret = av_find_best_stream(format_context_,  AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0)) < 0) {
        LOGE("%s %d av_find_best_stream fail (%d)", __FUNCTION__, __LINE__, ret);
        break;
      }

      audio_stream_ = format_context_->streams[ret];
      AVCodec* codec = FFGetDecoder(audio_stream_->codecpar->codec_id);
      if (!codec) {
        LOGE("%s %d FFGetDecoder fail", __FUNCTION__, __LINE__);
        break;
      }

      LOGI("%s %d codec, name: %s long_name: %s class_name: %s",
           __FUNCTION__, __LINE__, codec->name, codec->long_name, codec->priv_class->class_name);

      if (!(audio_context_ = avcodec_alloc_context3(codec))) {
        LOGE("%s %d avcodec_alloc_context3 fail", __FUNCTION__, __LINE__);
        break;
      }

      if ((ret = avcodec_parameters_to_context(audio_context_, audio_stream_->codecpar)) < 0) {
        LOGE("%s %d avcodec_parameters_to_context fail (%d)", __FUNCTION__, __LINE__, ret);
        break;
      }

      audio_context_->thread_count = 2;

      if ((ret = avcodec_open2(audio_context_, codec, NULL)) < 0) {
        LOGE("%s %d avcodec_open2 fail (%d)", __FUNCTION__, __LINE__, ret);
        break;
      }

      audio_frame_ = av_frame_alloc();

      AVDictionaryEntry* tag = NULL;
      while (audio_stream_->metadata && (tag = av_dict_get(audio_stream_->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
        LOGD("%s %d metaData: %s = %s", __FUNCTION__, __LINE__, tag->key, tag->value);
      }
    }

    if (!abort_video) {
      if ((ret = av_find_best_stream(format_context_, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0)) < 0) {
        LOGE("%s %d av_find_best_stream fail (%d)", __FUNCTION__, __LINE__, ret);
        break;
      }

      video_stream_ = format_context_->streams[ret];

      AVCodec* codec = FFGetDecoder(video_stream_->codecpar->codec_id);
      if (!codec) {
        LOGE("%s %d FFGetDecoder fail", __FUNCTION__, __LINE__);
        break;
      }

      LOGI("%s %d codec, name: %s long_name: %s class_name: %s",
           __FUNCTION__, __LINE__, codec->name, codec->long_name, codec->priv_class->class_name);

      if (!(video_context_ = avcodec_alloc_context3(codec))) {
        LOGE("%s %d avcodec_alloc_context3 fail", __FUNCTION__, __LINE__);
        break;
      }

      if ((ret = avcodec_parameters_to_context(video_context_, video_stream_->codecpar)) < 0) {
        LOGE("%s %d avcodec_parameters_to_context fail (%d)", __FUNCTION__, __LINE__, ret);
        break;
      }

      video_context_->thread_count = 2;

      if ((ret = avcodec_open2(video_context_, codec, NULL)) < 0) {
        LOGE("%s %d avcodec_open2 fail (%d)", __FUNCTION__, __LINE__, ret);
        break;
      }

      video_frame_ = av_frame_alloc();

      AVDictionaryEntry* tag = NULL;
      while (video_stream_->metadata && (tag = av_dict_get(video_stream_->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
        LOGI("%s %d metaData: %s = %s", __FUNCTION__, __LINE__, tag->key, tag->value);
      }
    }

    AVDictionaryEntry* tag = NULL;
    while (format_context_->metadata && (tag = av_dict_get(format_context_->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
      LOGI("%s %d metaData: %s = %s", __FUNCTION__, __LINE__, tag->key, tag->value);
    }

    av_dump_format(format_context_, 0, file_path, 0);

    inited_ = true;
    return true;
  } while (false);
  return false;
}

bool FFDecoder::Progress() {
  do {
    if (!inited_) {
      break;
    }

    int ret = 0;
    AVPacket pkt;
    av_init_packet(&pkt);
    if ((ret = av_read_frame(format_context_, &pkt)) != 0) {
      LOGE("%s %d av_read_frame fail (%d %s)", __FUNCTION__, __LINE__, ret, av_err2str(ret));
      break;
    }

    bool is_video = video_stream_ && pkt.stream_index == video_stream_->index;
    AVCodecContext* context = is_video ? video_context_ : audio_context_;
    AVStream* stream = is_video ? video_stream_ : audio_stream_;
    AVFrame* frame = is_video ? video_frame_ : audio_frame_;

//    LOGD("av_read_frame pts: %lld dts: %lld duration: %lld flags: %d idx: %d (%s %p %p %p)",
//         pkt.pts, pkt.dts, pkt.duration, pkt.flags, pkt.stream_index, is_video ? "video" : "audio",
//         context, stream, frame);
    if (stream) {
      Decode(&pkt, context, frame);
    }

    av_packet_unref(&pkt);

    return true;
  } while (false);
  return false;
}

bool FFDecoder::Seek(int64_t ts) {
  do {
    if (!inited_) {
      break;
    }

    AVStream* stream = video_stream_ ? video_stream_ : audio_stream_;
    if (stream == NULL) {
      break;
    }

    AVRational time_base_q;
    time_base_q.num = 1;
    time_base_q.den = AV_TIME_BASE;
    //     int64_t seekPos1 = av_rescale_q(stamp, time_base_q, stream->time_base);
    int64_t seekPos2 = av_rescale(ts, stream->time_base.den, AV_TIME_BASE * stream->time_base.num);

    int64_t pos = seekPos2;

    if (av_seek_frame(format_context_, stream->index, pos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME) < 0) {
      break;
    }

    return true;
  } while (false);

  return false;
}

bool FFDecoder::Flush() {
  if (!inited_) {
    return false;
  }

  Decode(nullptr, audio_context_, audio_frame_);
  Decode(nullptr, video_context_, video_frame_);
  return true;
}

void FFDecoder::Uint() {
  if (audio_frame_) {
    av_frame_free(&audio_frame_);
    audio_frame_ = nullptr;
  }

  if (video_frame_) {
    av_frame_free(&video_frame_);
    video_frame_ = nullptr;
  }

  if (audio_context_) {
    avcodec_free_context(&audio_context_);
    audio_context_ = nullptr;
  }

  if (video_context_) {
    avcodec_free_context(&video_context_);
    video_context_ = nullptr;
  }

  if (format_context_) {
    avformat_close_input(&format_context_);
    format_context_ = nullptr;
  }

  inited_ = false;
}

bool FFDecoder::Decode(AVPacket* pkt, AVCodecContext* context, AVFrame* frame) {
  if (!context || !frame) {
    return false;
  }

  int ret = 0;
  if ((ret = avcodec_send_packet(context, pkt)) != 0) {
    LOGE("%s %d avcodec_send_packet fail (%d %s)", __FUNCTION__, __LINE__, ret, av_err2str(ret));
    return false;
  }

  bool is_video = context->codec_type == AVMEDIA_TYPE_VIDEO;

  while (true) {
    if ((ret = avcodec_receive_frame(context, frame)) != 0) {
//      LOGE("%s %d avcodec_receive_frame fail (%d %s)", __FUNCTION__, __LINE__, ret, av_err2str(ret));
      break;
    }

    LOGD("%s %d %s format: %d pts: %d key_frame: %d", __FUNCTION__, __LINE__,
         is_video ? "video" : "audio", frame->format, frame->pts, frame->key_frame);
  }

  return true;
}