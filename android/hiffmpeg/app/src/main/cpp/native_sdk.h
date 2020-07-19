//
// Created by cort xu on 2021/9/11.
//

#ifndef HIFFMPEG_NATIVE_SDK_H
#define HIFFMPEG_NATIVE_SDK_H

#include <stdint.h>
#include <string>
#include <jni.h>

class NativeSdk {
public:
  NativeSdk();
  ~NativeSdk();

public:
  void set_enablehardware(bool enable);
  void set_enablemultithread(bool enable);
  void set_enablevideo(bool enable);
  void set_enableaudio(bool enable);
  void set_encodeparmas(int count, int width, int height, int fps, int bitrate);
  void set_basedir(const std::string& base_dir);

public:
  void ReParseH264(const std::string file_path);
  void TestBenchMark();
  void TestNativeEncode(std::string file_path);
  void TestNativeDecode(std::string file_path);
  void TestFFEncode(std::string file_path);
  void TestFFDecode(std::string file_path, std::string output_dir);
  void StartPlay(std::string file_path, jobject surface, int surface_width, int surface_height);
  void StopPlay();

private:
  bool        enable_hardware_;
  bool        enable_multithread_;
  bool        enable_video_;
  bool        enable_audio_;
  int         encode_count_;
  int         encode_width_;
  int         encode_height_;
  int         encode_fps_;
  int         encode_bitrate_;
  std::string base_dir_;
};


#endif //HIFFMPEG_NATIVE_SDK_H
