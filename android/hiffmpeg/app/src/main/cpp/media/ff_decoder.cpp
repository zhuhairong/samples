//
// Created by cort xu on 2021/8/21.
//

#include "ff_decoder.h"
#include "utils.h"
#include "media_codec.h"
#include "../jni_helper.h"
#include "bmp.h"
#include <sstream>

FFDecoder::FFDecoder(bool hardware, bool multithread)
  : hardware_(hardware), multithread_(multithread), inited_(false), video_frame_idx_(0), audio_frame_idx_(0), fp_audio_(NULL),
    format_context_(nullptr), audio_context_(nullptr), video_context_(nullptr),
    audio_stream_(nullptr), video_stream_(nullptr), audio_frame_(nullptr), video_frame_(nullptr) {
}

FFDecoder::~FFDecoder() {
  Uint();
}

bool FFDecoder::Init(const FFDecoderInfo& info) {
  do {
    if (inited_) {
      LOGD("%s %d dumplicate", __FUNCTION__, __LINE__);
      break;
    }

    if (!info.enable_video && !info.enable_audio) {
      break;
    }

    stats_.start_stamp = GetTickCount();

    int ret = avformat_open_input(&format_context_, info.file_path.c_str(), nullptr, nullptr);
    if (ret < 0) {
      LOGE("%s %d avformat_open_input fail (%d %s)", __FUNCTION__, __LINE__, ret, av_err2str(ret));
      break;
    }

    if ((ret = avformat_find_stream_info(format_context_, NULL)) < 0) {
      LOGE("%s %d avformat_find_stream_info fail (%d)", __FUNCTION__, __LINE__, ret);
      break;
    }

    LOGI("%s %d avformat_find_stream_info", __FUNCTION__, __LINE__);

    if (info.enable_audio) {
      LOGD("%s %d", __FUNCTION__, __LINE__);
      if ((ret = av_find_best_stream(format_context_,  AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0)) < 0) {
        LOGE("%s %d av_find_best_stream fail (%d)", __FUNCTION__, __LINE__, ret);
        break;
      }

      audio_stream_ = format_context_->streams[ret];
      LOGD("%s %d ret: %d stream: %p codecpar: %p codec_id: %d %d", __FUNCTION__, __LINE__, ret, audio_stream_, audio_stream_->codecpar, audio_stream_->codecpar->codec_id, AV_CODEC_ID_MP3);
      AVCodec* codec = FFGetDecoder(audio_stream_->codecpar->codec_id, hardware_);
      LOGD("%s %d", __FUNCTION__, __LINE__);
      if (!codec) {
        LOGE("%s %d FFGetDecoder fail", __FUNCTION__, __LINE__);
        break;
      }

      LOGD("%s %d ret: %d name: %s", __FUNCTION__, __LINE__, ret, codec->name);

//      LOGI("%s %d codec, name: %s long_name: %s class_name: %s",
//           __FUNCTION__, __LINE__, codec->name, codec->long_name, codec->priv_class->class_name);

      if (!(audio_context_ = avcodec_alloc_context3(codec))) {
        LOGE("%s %d avcodec_alloc_context3 fail", __FUNCTION__, __LINE__);
        break;
      }

      if ((ret = avcodec_parameters_to_context(audio_context_, audio_stream_->codecpar)) < 0) {
        LOGE("%s %d avcodec_parameters_to_context fail (%d)", __FUNCTION__, __LINE__, ret);
        break;
      }

      audio_context_->thread_count = multithread_ ? av_cpu_count() : 1;

      if (!info.output_dir.empty()) {
        std::string audio_path = info.output_dir + "/output.pcm";

//        fp_audio_ = fopen(audio_path.c_str(), "wb");
        LOGI("%s %d fopen %s", __FUNCTION__, __LINE__, audio_path.c_str());
      }

      av_opt_set_int(audio_context_->priv_data, "in_timeout", MEDIACODEC_INPUT_TIMEOUT_USEC, 0);
      av_opt_set_int(audio_context_->priv_data, "ou_timeout", MEDIACODEC_OUTPUT_TIMEOUT_USEC, 0);
      av_opt_set_int(audio_context_->priv_data, "in_timeout_times", 5, 0);
      av_opt_set_int(audio_context_->priv_data, "ou_timeout_times", 2, 0);

      if ((ret = avcodec_open2(audio_context_, codec, NULL)) < 0) {
        LOGE("%s %d avcodec_open2 fail (%d %s)", __FUNCTION__, __LINE__, ret, av_err2str(ret));
        break;
      }

      audio_frame_ = av_frame_alloc();

      AVDictionaryEntry* tag = NULL;
      while (audio_stream_->metadata && (tag = av_dict_get(audio_stream_->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
        LOGD("%s %d metaData: %s = %s", __FUNCTION__, __LINE__, tag->key, tag->value);
      }
    }

    if (info.enable_video) {
      if ((ret = av_find_best_stream(format_context_, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0)) < 0) {
        LOGE("%s %d av_find_best_stream fail (%d)", __FUNCTION__, __LINE__, ret);
        break;
      }

      video_stream_ = format_context_->streams[ret];

      AVCodec* codec = FFGetDecoder(video_stream_->codecpar->codec_id, hardware_);
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

      video_context_->thread_count = multithread_? 4 : 1;
//      av_opt_set_int(video_context_->priv_data, "in_timeout", MEDIACODEC_INPUT_TIMEOUT_USEC, 0);
//      av_opt_set_int(video_context_->priv_data, "ou_timeout", MEDIACODEC_OUTPUT_TIMEOUT_USEC, 0);
//      av_opt_set_int(video_context_->priv_data, "in_timeout_times", 5, 0);
//      av_opt_set_int(video_context_->priv_data, "ou_timeout_times", 2, 0);
      av_opt_set_int(video_context_->priv_data, "fill_extradata", 1, 0);

      if ((ret = avcodec_open2(video_context_, codec, NULL)) < 0) {
        LOGE("%s %d avcodec_open2 fail (%d %s)", __FUNCTION__, __LINE__, ret, av_err2str(ret));
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

    av_dump_format(format_context_, 0, info.file_path.c_str(), 0);

    inited_ = true;
  } while (false);

  LOGI("%s %d init %s", __FUNCTION__, __LINE__, inited_ ? "success" : "fail");
  return inited_;
}

bool FFDecoder::Process() {
  srand((unsigned )time(NULL));

  if (!inited_) {
    return false;
  }

  do {
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

    if (stream) {
      int64_t pts = (int64_t)ff_stamp2us(pkt.pts, stream->time_base);
      int64_t dts = (int64_t)ff_stamp2us(pkt.dts, stream->time_base);
      int64_t duration = (int64_t)ff_stamp2us(pkt.duration, stream->time_base);

      LOGD("av_read_frame %s pts: [%lld %lld] dts: [%lld %lld] duration: [%lld %lld] flags: %d idx: %d data: [%x %x %x %x %x]", is_video ? "video" : "audio",
           pkt.pts, pts, pkt.dts, dts, pkt.duration, duration, pkt.flags, pkt.stream_index, pkt.data[0], pkt.data[1], pkt.data[2], pkt.data[3], pkt.data[4]);

      pkt.pts = pts;
      pkt.dts = dts;
      pkt.duration = duration;

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
    int ret = 0;
    if ((ret = av_seek_frame(format_context_, stream->index, pos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME)) < 0) {
      LOGD("av_seek_frame fail (%d)", ret);
      break;
    }

    LOGD("av_seek_frame ts: %lld", ts);
    return true;
  } while (false);

  return false;
}

bool FFDecoder::Flush() {
  if (!inited_) {
    return false;
  }

  LOGD("%s %d", __FUNCTION__, __LINE__);
  Decode(nullptr, audio_context_, audio_frame_);
  Decode(nullptr, video_context_, video_frame_);
  LOGD("%s %d", __FUNCTION__, __LINE__);
  return true;
}

void FFDecoder::Uint() {
  if (inited_) {
    stats_.end_stamp = GetTickCount();

    uint32_t spent = stats_.end_stamp - stats_.start_stamp;
    LOGD("%s %d spent: %u audio: [s: %u r: %u] video: [s: %u r: %u]", __FUNCTION__, __LINE__,
         spent, stats_.audio_sent_cnt, stats_.audio_recv_cnt, stats_.video_sent_cnt, stats_.video_recv_cnt);
  }

  video_frame_idx_ = 0;
  audio_frame_idx_ = 0;

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
//    int64_t in_timeout = 0;
//    av_opt_get_int(video_context_->priv_data, "in_timeout", 0, &in_timeout);
//
//    int64_t get_frame_cnt = 0;
//    av_opt_get_int(video_context_->priv_data, "get_frame_cnt", 0, &get_frame_cnt);

    LOGD("%s thread_count: %d cpu: %d", __FUNCTION__, video_context_->thread_count, av_cpu_count());

    avcodec_free_context(&video_context_);
    video_context_ = nullptr;
  }

  if (format_context_) {
    avformat_close_input(&format_context_);
    format_context_ = nullptr;
  }

  if (fp_audio_) {
    fclose(fp_audio_);
    fp_audio_ = NULL;
  }

  inited_ = false;
}

bool FFDecoder::Decode(AVPacket* pkt, AVCodecContext* context, AVFrame* frame) {
  if (!context || !frame) {
    return false;
  }

//  if (pkt) {
//    LOGD("%s %d data: %p size: %d flags: %d pos: %lld pts: %lld dts: %lld duration: %lld", __FUNCTION__, __LINE__,
//         pkt->data, pkt->size, pkt->flags, pkt->pos, pkt->pts, pkt->dts, pkt->duration);
//  }

  bool is_video = context->codec_type == AVMEDIA_TYPE_VIDEO;
  auto& frame_idx = is_video ? video_frame_idx_ : audio_frame_idx_;
  auto& sent_cnt = is_video ? stats_.video_sent_cnt : stats_.audio_sent_cnt;
  auto& recv_cnt = is_video ? stats_.video_recv_cnt : stats_.audio_recv_cnt;
  uint32_t count = 0;

  static uint32_t sent_count = 0;
  static uint32_t recv_count = 0;
  static uint32_t decode_count = 0;
  static uint64_t decode_spent = 0;

  int again_error = AVERROR(EAGAIN);

  uint64_t start_stamp = GetTickCount();
  if (!pkt) {
    LOGE("%s %d avcodec_send_packet flush %s", __FUNCTION__, __LINE__, is_video ? "video" : "audio");
  }

  int ret = 0;
  if ((ret = avcodec_send_packet(context, pkt)) != 0) {
    LOGE("%s %d avcodec_send_packet fail (%d %s)", __FUNCTION__, __LINE__, ret, av_err2str(ret));
    return false;
  }

  ++ sent_cnt;
  ++ sent_count;

  while (true) {
    ret = avcodec_receive_frame(context, frame);
    if (ret != 0) {
      if (ret == again_error) {
        LOGE("%s %d avcodec_receive_frame fail (%d %s)", __FUNCTION__, __LINE__, ret, av_err2str(ret));
      }
      break;
    }

    ++ recv_cnt;
    ++ recv_count;

    uint32_t frame_data_size = is_video ?  av_image_get_buffer_size((enum AVPixelFormat)frame->format, frame->width, frame->height, 1) : av_samples_get_buffer_size(NULL, frame->channels, frame->nb_samples, (enum AVSampleFormat)frame->format, 1);

    LOGD("%s %s format: %d flags: %d nb_samples: %d frame_data_size: %d pts: %lld dts: %lld duration: %lld width: %d height: %d channel: %d samplerate: %d crop: [%d %d %d %d]", __FUNCTION__,
         is_video ? "video" : "audio", frame->format, frame->flags, frame->nb_samples, frame_data_size, frame->pts, frame->pkt_dts, frame->pkt_duration,
         frame->width, frame->height, frame->channels, frame->sample_rate,
         frame->crop_left, frame->crop_top, frame->crop_bottom, frame->crop_bottom);

//    if (!is_video) {
//      if (fp_audio_) {
//        fwrite(frame->data[0], 1, frame_data_size, fp_audio_);
//      }
//      continue;
//    }

    uint64_t now = GetTickCount();
    decode_spent += (now - start_stamp);
    ++ decode_count;

    static uint64_t start_frame_stamp = frame->pts;
    static uint64_t start_render_stamp = now;

    ++ frame_idx;
  }

  return true;
}