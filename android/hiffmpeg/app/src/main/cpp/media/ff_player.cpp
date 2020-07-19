//
// Created by cort xu on 2021/8/21.
//

#include "ff_player.h"
#include "utils.h"
#include "../jni_helper.h"
#include "bmp.h"
#include <sstream>

FFPlayer::FFPlayer(bool hardware)
  : hardware_(hardware), inited_(false), video_frame_idx_(0), audio_frame_idx_(0), native_window_(nullptr),
    format_context_(nullptr), audio_context_(nullptr), video_context_(nullptr),
    audio_stream_(nullptr), video_stream_(nullptr), audio_frame_(nullptr), video_frame_(nullptr), rgb_frame_(nullptr), sws_ctx_(nullptr) {
}

FFPlayer::~FFPlayer() {
  Uint();
}

bool FFPlayer::Init(JNIEnv *env, jstring input, jstring  output_path, jobject surface, const FFPlayerInfo& info) {
  do {
    if (inited_) {
      LOGD("%s %d dumplicate", __FUNCTION__, __LINE__);
      break;
    }

    native_window_ = ANativeWindow_fromSurface(env, surface);
    if (!native_window_) {
      LOGD("%s %d ANativeWindow_fromSurface fail (%p %p %p)", __FUNCTION__, __LINE__, env, surface, native_window_);
      break;
    }

    std::string input_file;
    JNIHelper::Jstring2Str(env, input, input_file);

    JNIHelper::Jstring2Str(env, output_path, output_path_);

    if (!info.audio_enable && !info.video_enable) {
      break;
    }

    int ret = avformat_open_input(&format_context_, input_file.c_str(), nullptr, nullptr);
    if (ret < 0) {
      LOGE("%s %d avformat_open_input fail (%d %s)", __FUNCTION__, __LINE__, ret, av_err2str(ret));
      break;
    }

    if ((ret = avformat_find_stream_info(format_context_, NULL)) < 0) {
      LOGE("%s %d avformat_find_stream_info fail (%d)", __FUNCTION__, __LINE__, ret);
      break;
    }

    if (info.audio_enable) {
      if ((ret = av_find_best_stream(format_context_,  AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0)) < 0) {
        LOGE("%s %d av_find_best_stream fail (%d)", __FUNCTION__, __LINE__, ret);
        break;
      }

      audio_stream_ = format_context_->streams[ret];
      AVCodec* codec = FFGetDecoder(audio_stream_->codecpar->codec_id, hardware_);
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

    if (info.video_enable) {
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

      video_context_->thread_count = 4;

      if ((ret = avcodec_open2(video_context_, codec, NULL)) < 0) {
        LOGE("%s %d avcodec_open2 fail (%d)", __FUNCTION__, __LINE__, ret);
        break;
      }

      video_frame_ = av_frame_alloc();
      rgb_frame_ = av_frame_alloc();
      rgb_frame_->format = AV_PIX_FMT_RGBA;
      rgb_frame_->width = video_context_->width;
      rgb_frame_->height = video_context_->height;

//      sws_ctx_ = sws_getContext(video_context_->width, video_context_->height,
//                                video_context_->pix_fmt,
//                                rgb_frame_->width, rgb_frame_->height, AV_PIX_FMT_RGBA,
//                                SWS_BICUBIC, NULL, NULL, NULL);

      uint8_t* rgb_buff = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_RGBA, rgb_frame_->width, rgb_frame_->height));
      avpicture_fill((AVPicture *)rgb_frame_, rgb_buff,AV_PIX_FMT_RGBA, rgb_frame_->width, rgb_frame_->height);

      AVDictionaryEntry* tag = NULL;
      while (video_stream_->metadata && (tag = av_dict_get(video_stream_->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
        LOGI("%s %d metaData: %s = %s", __FUNCTION__, __LINE__, tag->key, tag->value);
      }
    }

    AVDictionaryEntry* tag = NULL;
    while (format_context_->metadata && (tag = av_dict_get(format_context_->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
      LOGI("%s %d metaData: %s = %s", __FUNCTION__, __LINE__, tag->key, tag->value);
    }

    av_dump_format(format_context_, 0, input_file.c_str(), 0);

    inited_ = true;
  } while (false);

  LOGI("%s %d init %s", __FUNCTION__, __LINE__, inited_ ? "success" : "fail");
  return inited_;
}

void FFPlayer::Process() {
  srand((unsigned )time(NULL));

  if (!inited_) {
    return;
  }

  while (true) {
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
      pkt.pts = (int64_t)ff_stamp2us(pkt.pts, stream->time_base);
      pkt.dts = (int64_t)ff_stamp2us(pkt.dts, stream->time_base);
      pkt.duration = (int64_t)ff_stamp2us(pkt.duration, stream->time_base);

      LOGD("av_read_frame %s pts: %lld dts: %lld duration: %lld flags: %d idx: %d data: [%x %x %x %x %x]", is_video ? "video" : "audio",
           pkt.pts, pkt.dts, pkt.duration, pkt.flags, pkt.stream_index, pkt.data[0], pkt.data[1], pkt.data[2], pkt.data[3], pkt.data[4]);

      Decode(&pkt, context, frame);
    }

    av_packet_unref(&pkt);
  }

  Flush();
}

bool FFPlayer::Seek(int64_t ts) {
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

bool FFPlayer::Flush() {
  if (!inited_) {
    return false;
  }

  Decode(nullptr, audio_context_, audio_frame_);
  Decode(nullptr, video_context_, video_frame_);
  return true;
}

void FFPlayer::Uint() {
  inited_ = false;

  if (native_window_) {
    ANativeWindow_release(native_window_);
    native_window_ = nullptr;
  }

  if (audio_frame_) {
    av_frame_free(&audio_frame_);
    audio_frame_ = nullptr;
  }

  if (video_frame_) {
    av_frame_free(&video_frame_);
    video_frame_ = nullptr;
  }

  if (rgb_frame_) {
    av_frame_free(&rgb_frame_);
    rgb_frame_ = nullptr;
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
}

bool FFPlayer::Decode(AVPacket* pkt, AVCodecContext* context, AVFrame* frame) {
  if (!context || !frame) {
    return false;
  }

//  if (pkt) {
//    LOGD("%s %d data: %p size: %d flags: %d pos: %lld pts: %lld dts: %lld duration: %lld", __FUNCTION__, __LINE__,
//         pkt->data, pkt->size, pkt->flags, pkt->pos, pkt->pts, pkt->dts, pkt->duration);
//  }

  static uint32_t sent_count = 0;
  static uint32_t recv_count = 0;
  static uint32_t decode_count = 0;
  static uint64_t decode_spent = 0;

  uint64_t start_stamp = GetTickCount();
  int ret = 0;
  if ((ret = avcodec_send_packet(context, pkt)) != 0) {
    LOGE("%s %d avcodec_send_packet fail (%d %s)", __FUNCTION__, __LINE__, ret, av_err2str(ret));
    return false;
  }

  ++ sent_count;

  bool is_video = context->codec_type == AVMEDIA_TYPE_VIDEO;
  auto& frame_idx = is_video ? video_frame_idx_ : audio_frame_idx_;
  uint32_t count = 0;

  while (true) {
    if ((ret = avcodec_receive_frame(context, frame)) != 0) {
//      LOGE("%s %d avcodec_receive_frame fail (%d %s)", __FUNCTION__, __LINE__, ret, av_err2str(ret));
      break;
    }

    ++ recv_count;

    LOGD("%s %s format: %d flags: %d pts: %lld dts: %lld duration: %lld width: %d height: %d channel: %d samplerate: %d crop: [%d %d %d %d]", __FUNCTION__,
         is_video ? "video" : "audio", frame->format, frame->flags, frame->pts, frame->pkt_dts, frame->pkt_duration,
         frame->width, frame->height, frame->channels, frame->sample_rate,
         frame->crop_left, frame->crop_top, frame->crop_bottom, frame->crop_bottom);

    if (!is_video) {
      continue;
    }

    uint64_t now = GetTickCount();
    decode_spent += (now - start_stamp);
    ++ decode_count;

    static uint64_t start_frame_stamp = frame->pts;
    static uint64_t start_render_stamp = now;

    if (!sws_ctx_) {
      sws_ctx_ = sws_getContext(frame->width, frame->height,
                                (AVPixelFormat)frame->format,
                                rgb_frame_->width, rgb_frame_->height, AV_PIX_FMT_RGBA,
                                SWS_BICUBIC, NULL, NULL, NULL);
    }

    //转换为rgb格式
    int sws_scale_ret = sws_scale(sws_ctx_,(const uint8_t *const *)frame->data, frame->linesize,0, frame->height, rgb_frame_->data, rgb_frame_->linesize);


    ANativeWindow_setBuffersGeometry(native_window_, rgb_frame_->width, rgb_frame_->height,WINDOW_FORMAT_RGBA_8888);

    ANativeWindow_Buffer native_buffer;
    ANativeWindow_lock(native_window_, &native_buffer, NULL);
    //  rgb_frame是有画面数据
    uint8_t *dst= (uint8_t *) native_buffer.bits;
  //            拿到一行有多少个字节 RGBA
    int dstStride=native_buffer.stride * 4;
    //像素数据的首地址AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED
    uint8_t * src=  rgb_frame_->data[0];
  //            实际内存一行数量
    int srcStride = rgb_frame_->linesize[0];
    //int i=0;
    for (int i = 0; i < rgb_frame_->height; ++i) {
      memcpy(dst + i * dstStride,  src + i * srcStride, srcStride);
    }
  //解锁
    ANativeWindow_unlockAndPost(native_window_);

    ++ frame_idx;

    if (frame_idx % 30 == 0) {
//      std::ostringstream oss;
//      oss << output_path_ << "/" << frame_idx << ".bmp";
//      std::string file_path = oss.str();
//      uint32_t frame_data_size = av_image_get_buffer_size((enum AVPixelFormat)rgb_frame_->format, rgb_frame_->width, rgb_frame_->height, 1);
//      BmpEncode(src, frame_data_size, rgb_frame_->width, rgb_frame_->height, file_path);
//      LOGD("%s %d bmp: %s", __FUNCTION__, __LINE__, file_path.c_str());

      LOGD("%s %d stats, cnt: %u spent: %llu avg: %llu io: [%u %u] render: [%llu %llu]",
           __FUNCTION__, __LINE__, decode_count, decode_spent, decode_spent / decode_count, sent_count, recv_count, now - start_render_stamp, frame->pts - start_frame_stamp);
      LOGD("%s %d %s format: %d pts: %d key_frame: %d linesize: [%d %d] frame_size: [%d %d] pix_fmt: %d ctx_size: [%d %d %d %d %d] rgb_size: [%d %d]",
           __FUNCTION__, __LINE__,
           is_video ? "video" : "audio", frame->format, frame->pts, frame->key_frame, frame->linesize[0], frame->linesize[1],
           frame->width, frame->height, context->pix_fmt, context->width, context->height, context->coded_width, context->coded_height, context->lowres,
           rgb_frame_->width, rgb_frame_->height);
    }

//    usleep(1000 * 16);
  }

  return true;
}