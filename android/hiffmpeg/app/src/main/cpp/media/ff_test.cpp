//
// Created by cort xu on 2021/8/21.
//

#include "ff_test.h"
#include "utils.h"
#include "../jni_helper.h"

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

FFTest::FFTest()
  : inited_(false), native_window_(nullptr), format_context_(nullptr),
    audio_context_(nullptr), video_context_(nullptr),
    audio_stream_(nullptr), video_stream_(nullptr),
    audio_frame_(nullptr), video_frame_(nullptr), rgb_frame_(nullptr), sws_ctx_(nullptr) {

}

FFTest::~FFTest() {
  Uint();
}

bool FFTest::Init(JNIEnv *env, jstring path, jobject surface, bool abort_audio, bool abort_video) {
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

    std::string file_path;
    JNIHelper::Jstring2Str(env, path, file_path);

    if (abort_audio && abort_video) {
      break;
    }

    int ret = avformat_open_input(&format_context_, file_path.c_str(), nullptr, nullptr);
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
      rgb_frame_ = av_frame_alloc();

      uint8_t* rgb_buff = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_RGBA, video_context_->width, video_context_->height));
      avpicture_fill((AVPicture *)rgb_frame_, rgb_buff,AV_PIX_FMT_RGBA, video_context_->width, video_context_->height);

      sws_ctx_ = sws_getContext(video_context_->width,video_context_->height,video_context_->pix_fmt,
                                video_context_->width,video_context_->height,AV_PIX_FMT_RGBA,
                                SWS_BICUBIC,NULL,NULL,NULL);

      AVDictionaryEntry* tag = NULL;
      while (video_stream_->metadata && (tag = av_dict_get(video_stream_->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
        LOGI("%s %d metaData: %s = %s", __FUNCTION__, __LINE__, tag->key, tag->value);
      }
    }

    AVDictionaryEntry* tag = NULL;
    while (format_context_->metadata && (tag = av_dict_get(format_context_->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
      LOGI("%s %d metaData: %s = %s", __FUNCTION__, __LINE__, tag->key, tag->value);
    }

    av_dump_format(format_context_, 0, file_path.c_str(), 0);

    inited_ = true;
  } while (false);

  LOGI("%s %d init %s", __FUNCTION__, __LINE__, inited_ ? "success" : "fail");
  return inited_;
}

bool FFTest::Flush() {
  if (!inited_) {
    return false;
  }

  Decode(nullptr, audio_context_, audio_frame_);
  Decode(nullptr, video_context_, video_frame_);
  return true;
}

void FFTest::Uint() {
  inited_ = false;

  LOGI("%s %d", __FUNCTION__, __LINE__);

  if (native_window_) {
    ANativeWindow_release(native_window_);
    native_window_ = nullptr;
  }

  LOGI("%s %d", __FUNCTION__, __LINE__);

  if (audio_frame_) {
    av_frame_free(&audio_frame_);
    audio_frame_ = nullptr;
  }

  LOGI("%s %d", __FUNCTION__, __LINE__);

  if (video_frame_) {
    av_frame_free(&video_frame_);
    video_frame_ = nullptr;
  }

  LOGI("%s %d", __FUNCTION__, __LINE__);

  if (rgb_frame_) {
    av_frame_free(&rgb_frame_);
    rgb_frame_ = nullptr;
  }

  LOGI("%s %d", __FUNCTION__, __LINE__);

  if (audio_context_) {
    avcodec_free_context(&audio_context_);
    audio_context_ = nullptr;
  }

  LOGI("%s %d", __FUNCTION__, __LINE__);

  if (video_context_) {
    avcodec_free_context(&video_context_);//会清理 priv_data 和 extradata
    video_context_ = nullptr;
  }

  LOGI("%s %d", __FUNCTION__, __LINE__);

  if (format_context_) {
    avformat_close_input(&format_context_);
    format_context_ = nullptr;
  }
}

bool FFTest::Decode(AVPacket* pkt, AVCodecContext* context, AVFrame* frame) {
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
      LOGE("%s %d avcodec_receive_frame fail (%d %s)", __FUNCTION__, __LINE__, ret, av_err2str(ret));
      break;
    }

    LOGD("%s %d %s format: %d pts: %d key_frame: %d", __FUNCTION__, __LINE__,
         is_video ? "video" : "audio", frame->format, frame->pts, frame->key_frame);

    if (!is_video) {
      continue;
    }

    ANativeWindow_setBuffersGeometry(native_window_, context->width, context->height,WINDOW_FORMAT_RGBA_8888);

    ANativeWindow_lock(native_window_, &native_buffer_, NULL);
    //转换为rgb格式
    sws_scale(sws_ctx_,(const uint8_t *const *)frame->data, frame->linesize,0, frame->height, rgb_frame_->data, rgb_frame_->linesize);
    //  rgb_frame是有画面数据
    uint8_t *dst= (uint8_t *) native_buffer_.bits;
//            拿到一行有多少个字节 RGBA
    int destStride=native_buffer_.stride * 4;
    //像素数据的首地址
    uint8_t * src=  rgb_frame_->data[0];
//            实际内存一行数量
    int srcStride = rgb_frame_->linesize[0];
    //int i=0;
    for (int i = 0; i < context->height; ++i) {
      memcpy(dst + i * destStride,  src + i * srcStride, srcStride);
    }
//解锁
    ANativeWindow_unlockAndPost(native_window_);

    usleep(1000 * 16);
  }

  return true;
}