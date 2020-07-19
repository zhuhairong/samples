//
// Created by cort xu on 2021/9/13.
//

#ifndef HIFFMPEG_H264_SOURCES_H
#define HIFFMPEG_H264_SOURCES_H
#include "h264_nalu_parse.h"

class H264Sources {
public:
  static H264Sources* GetInstance();

private:
  H264Sources();
  ~H264Sources();

public:
  bool Get(const std::string& file_path, H264Info& info);
  void Parse(const std::string& file_path);

private:
  std::map<std::string, H264Info> infos;
};


#endif //HIFFMPEG_H264_SOURCES_H
