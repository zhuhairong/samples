//
// Created by cort xu on 2021/9/11.
//

#include "native_sdk.h"
#include "media/h264_sources.h"
#include "media/utils.h"
#include "media/video_encode.h"
#include "media/video_decode.h"
#include "media/ff_encoder.h"
#include "media/ff_decoder.h"

NativeSdk::NativeSdk()
    : enable_hardware_(false), enable_multithread_(false),
      enable_video_(false), enable_audio_(false), encode_width_(0), encode_height_(0), encode_fps_(0), encode_bitrate_(0) {
}

NativeSdk::~NativeSdk() {
}

void NativeSdk::set_enablehardware(bool enable) {
  enable_hardware_ = enable;

  LOGD("%s enable: %s", __FUNCTION__, enable ? "yes" : "no");
}

void NativeSdk::set_enablemultithread(bool enable) {
    enable_multithread_ = enable;
    LOGD("%s enable: %s", __FUNCTION__, enable ? "yes" : "no");
}

void NativeSdk::set_enablevideo(bool enable) {
  enable_video_ = enable;
}

void NativeSdk::set_enableaudio(bool enable) {
  enable_audio_ = enable;
}

void NativeSdk::set_encodeparmas(int count, int width, int height, int fps, int bitrate) {
  encode_count_ = count;
  encode_width_ = width;
  encode_height_ = height;
  encode_fps_ = fps;
  encode_bitrate_ = bitrate;

  LOGD("%s count: %d width: %d height: %d fps: %d bitrate: %d",
       __FUNCTION__, count, width, height, fps, bitrate);
}

void NativeSdk::set_basedir(const std::string& base_dir) {
  base_dir_ = base_dir;
}

void NativeSdk::ReParseH264(const std::string file_path) {
  H264Sources::GetInstance()->Parse(file_path);
}

void NativeSdk::TestBenchMark() {

}

void NativeSdk::TestNativeEncode(std::string file_path) {
  LOGD("%s %d filePath: %s", __FUNCTION__, __LINE__, file_path.c_str());

  CreateParentPath(file_path);

  do {
    VideoEncodeParmas parmas;
    parmas.width = encode_width_;
    parmas.height = encode_height_;
    parmas.fps = encode_fps_;
    parmas.bitrate = encode_bitrate_;
    parmas.frame_cnt = encode_count_;
    parmas.file_path = file_path;

    VideoEncode encode;
    if (!encode.Init(parmas)) {
      break;
    }

    encode.Progress();
    encode.Uint();
  } while (false);
}

void NativeSdk::TestNativeDecode(std::string file_path) {
  VideoDecode decoder;

  do {
    if (!decoder.Init(file_path, base_dir_)) {
      break;
    }

    decoder.Progress();
    decoder.Uint();
  } while (false);
}

void NativeSdk::TestFFEncode(std::string file_path) {

}

void NativeSdk::TestFFDecode(std::string file_path, std::string output_dir) {
  FFDecoder decoder(enable_hardware_, enable_multithread_);

  do {
    FFDecoderInfo info;
    info.file_path = file_path;
    info.output_dir = output_dir;
    info.enable_video = enable_video_;
    info.enable_audio = enable_audio_;
    if (!decoder.Init(info)) {
      break;
    }

    uint32_t count = 0;
    while (decoder.Process()) {
//      if (++ count % 300 == 0) {
//        decoder.Seek(0);
//      }
    }
    decoder.Flush();
  } while (false);

  decoder.Uint();
}

void NativeSdk::StartPlay(std::string file_path, jobject surface, int surface_width, int surface_height) {

}

void NativeSdk::StopPlay() {
}