//
// Created by cort xu on 2021/9/6.
//

#include "ff_encoder.h"

FFEncoder::FFEncoder() {
}

FFEncoder::~FFEncoder() {
  Uint();
}

bool FFEncoder::Init(const FFEncoderInfo& info) {

  do {
    if (inited_) {
      LOGE("%s %d init dumplicate failed", __FUNCTION__, __LINE__);
      break;
    }

    if (!info.audio_enable && !info.video_enable) {
      LOGE("%s %d not enable", __FUNCTION__, __LINE__);
      break;
    }

    int ret = 0;
    if ((ret = avformat_alloc_output_context2(&format_context_, NULL, NULL, info.file_path.c_str())) < 0) {
      LOGE("%s %d avformat_alloc_output_context2 fail (%d %s)", __FUNCTION__, __LINE__, ret, av_err2str(ret));
      break;
    }

    if (info.video_enable) {
      AVCodec* codec = FFGetEncoder(AV_CODEC_ID_H264, true);
      if (codec == NULL) {
        LOGE("%s %d FFGetEncoder fail", __FUNCTION__, __LINE__);
        break;
      }

      if (!(video_context_ = avcodec_alloc_context3(codec))) {
        LOGE("%s %d avcodec_alloc_context3 fail", __FUNCTION__, __LINE__);
        break;
      }

      video_context_->codec_id = codec->id;
      video_context_->qmin = 15;
      video_context_->qmax = 20;
      video_context_->bit_rate = info.video_bitrate;
#ifndef HILIVE_SYS_IOS// ios 加上码率限制画面会模糊，原因待查
      video_context_->rc_max_rate = info.video_bitrate * 5;
      video_context_->rc_min_rate = info.video_bitrate * 0.5;
#endif
      video_context_->time_base = (AVRational){1, (int)info.video_fps};
      video_context_->framerate = (AVRational){(int)info.video_fps, 1};
      video_context_->width = info.video_width;
      video_context_->height = info.video_height;
      video_context_->pix_fmt = info.video_format;
      video_context_->gop_size = info.video_gop > 0 ? info.video_gop : info.video_fps;
      video_context_->max_b_frames = 0;
      video_context_->thread_count = 2;

      if (codec->id == AV_CODEC_ID_H264) {
        video_context_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
      }

      if ((ret = avcodec_open2(video_context_, codec, NULL)) < 0) {
        LOGE("%s %d avcodec_open2 fail", __FUNCTION__, __LINE__);
        break;
      }

      video_frame_duration_ = (int32_t)ff_stamp2us(1, video_context_->time_base);

      video_frame_ = av_frame_alloc();
      video_frame_->pts = 0;
      video_frame_->format = video_context_->pix_fmt;
      video_frame_->width = video_context_->width;
      video_frame_->height = video_context_->height;

      if (!(video_stream_ = avformat_new_stream(format_context_, NULL))) {
        LOGE("%s %d avformat_new_stream fail", __FUNCTION__, __LINE__);
        break;
      }

      if ((ret = avcodec_parameters_from_context(video_stream_->codecpar, video_context_)) < 0) {
        LOGE("%s %d avcodec_parameters_from_context fail (%d)", __FUNCTION__, __LINE__, ret);
        break;
      }
    }

    if (info.audio_enable) {
      AVCodec* codec = FFGetEncoder(AV_CODEC_ID_AAC, false);
      if (codec == NULL) {
        LOGE("%s %d FFGetEncoder fail", __FUNCTION__, __LINE__);
        break;
      }

      if (!(audio_context_ = avcodec_alloc_context3(codec))) {
        LOGE("%s %d avcodec_alloc_context3 fail", __FUNCTION__, __LINE__);
        break;
      }

      audio_context_->codec_id = codec->id;
      audio_context_->bit_rate = info.audio_bitrate;
      audio_context_->sample_fmt = info.audio_format;
      audio_context_->sample_rate = info.audio_samplerate;
      audio_context_->time_base = (AVRational){1, (int)info.audio_samplerate};
      audio_context_->channels = info.audio_channel;
      audio_context_->channel_layout = av_get_default_channel_layout(audio_context_->channels);
      audio_context_->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
      audio_context_->thread_count = 2;

      if ((ret = avcodec_open2(audio_context_, codec, NULL)) < 0) {
        LOGE("%s %d avcodec_open2 fail (%d)", __FUNCTION__, __LINE__, ret);
        break;
      }

      if (audio_context_->frame_size <= 0) {
        audio_context_->frame_size = 1024;
      }

      audio_frame_duration_ = (uint32_t)ff_stamp2us(audio_context_->frame_size, audio_context_->time_base);

      audio_frame_ = av_frame_alloc();
      audio_frame_->pts = 0;
      audio_frame_->nb_samples = audio_context_->frame_size;
      audio_frame_->channels = audio_context_->channels;
      audio_frame_->channel_layout = audio_context_->channel_layout;
      audio_frame_->format = audio_context_->sample_fmt;
      audio_frame_->sample_rate = audio_context_->sample_rate;

      if (!(audio_stream_ = avformat_new_stream(format_context_, NULL))) {
        LOGE("%s %d avformat_new_stream fail", __FUNCTION__, __LINE__);
        break;
      }

      if ((ret = avcodec_parameters_from_context(audio_stream_->codecpar, audio_context_)) < 0) {
        LOGE("%s %d avcodec_parameters_from_context fail (%d)", __FUNCTION__, __LINE__, ret);
        break;
      }
    }

    av_dump_format(format_context_, 0, info.file_path.c_str(), 1);

    if (!(format_context_->oformat->flags & AVFMT_NOFILE) &&
        (ret = avio_open(&format_context_->pb, info.file_path.c_str(), AVIO_FLAG_WRITE)) < 0) {

      LOGE("%s %d avio_open fail", __FUNCTION__, __LINE__);
      break;
    }

    if (avformat_write_header(format_context_, NULL) < 0) {
      LOGE("%s %d avformat_write_header fail", __FUNCTION__, __LINE__);
      break;
    }

    LOGI("%s frameDuration: [a: %u v: %u]", __FUNCTION__, audio_frame_duration_, video_frame_duration_);
    inited_ = true;
  } while (false);

  return inited_;
}

bool FFEncoder::EncodeVideo(uint8_t* data, uint32_t size, int64_t pts) {
  if (!video_context_ || !video_frame_) {
    return false;
  }

  video_frame_->pts = pts;

  int ret = 0;
  if ((ret = av_image_fill_arrays(video_frame_->data, video_frame_->linesize, data, video_context_->pix_fmt, video_context_->width, video_context_->height, 1)) < 0) {
    return false;
  }

  if (!Encode(video_context_, video_frame_)) {
    return false;
  }

  return true;
}

bool FFEncoder::EncodeAudio(uint8_t* data, uint32_t size, int64_t pts) {
  if (!audio_context_ || !audio_frame_) {
    return false;
  }

  audio_frame_->pts = pts;

  int ret = 0;
  if ((ret = avcodec_fill_audio_frame(audio_frame_, audio_context_->channels, audio_context_->sample_fmt, data, size, 1)) < 0) {
    return false;
  }

  if (!Encode(audio_context_, audio_frame_)) {
    return false;
  }

  return true;
}

void FFEncoder::Uint() {
  if (format_context_) {
    if (inited_) {
      av_write_trailer(format_context_);
    }

    if (format_context_->flags & AVFMT_NOFILE) {
      avio_close(format_context_->pb);
    }

    avformat_close_input(&format_context_);
    format_context_ = nullptr;
  }

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

  inited_ = false;
  audio_frame_duration_ = 0;
  video_frame_duration_ = 0;
}

bool FFEncoder::Encode(AVCodecContext* context, AVFrame* frame) {
  if (context == NULL) {
    return false;
  }

  bool is_video = context->codec_type == AVMEDIA_TYPE_VIDEO;
  bool flush_frame = frame ? false : true;

  AVStream* stream = is_video ? video_stream_ : audio_stream_;

  int ret = 0;
  if ((ret = avcodec_send_frame(context, frame)) != 0) {//while frame == NULL, Flush encoder
    return false;
  }

  while (true) {
    AVPacket pkt;
    av_init_packet(&pkt);
    if ((ret = avcodec_receive_packet(context, &pkt)) != 0) {
      break;
    }

    double pts = ff_stamp2us_q(pkt.pts, stream->time_base);
    double dts = ff_stamp2us_q(pkt.dts, stream->time_base);
    double duration = ff_stamp2us_q(pkt.duration, stream->time_base);

    pkt.stream_index = stream->index;
    pkt.pts = pts;
    pkt.dts = dts;
    pkt.duration = duration;//time to stream stamp

    if ((ret = av_interleaved_write_frame(format_context_, &pkt)) < 0) {
      av_packet_unref(&pkt);
      return false;
    }

    av_packet_unref(&pkt);
  }

  return true;
}