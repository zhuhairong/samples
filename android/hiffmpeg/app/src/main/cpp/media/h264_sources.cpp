//
// Created by cort xu on 2021/9/13.
//

#include "h264_sources.h"
#include "utils.h"

H264Sources* H264Sources::GetInstance() {
  static H264Sources* sources = nullptr;
  static std::once_flag onceflag;
  std::call_once(onceflag, [&] {
    sources = new H264Sources();
  });

  return sources;
}

H264Sources::H264Sources() {

}

H264Sources::~H264Sources() {

}

bool H264Sources::Get(const std::string& file_path, H264Info& info) {
  auto it = infos.find(file_path);
  if (it != infos.end()) {
    info = it->second;
    LOGD("%s got file", __FUNCTION__);
    return true;
  } else {
    LOGD("%s got fail", __FUNCTION__);
    return false;
  }
}

void H264Sources::Parse(const std::string& file_path) {
  FILE* fp = nullptr;
  do {
    if (infos.find(file_path) != infos.end()) {
      break;
    }

    H264Info info;
    fp = fopen(file_path.c_str(), "rb");
    H264NaluParse h264NaluParse;
    while (h264NaluParse.parse(fp, info)) {}

    infos[file_path] = info;

    LOGD("%s load file complete width: %d height: %d fps: %d nalus: %u", __FUNCTION__,
         info.width, info.height, info.fps, info.nalus.size());
  } while (false);

  if (fp) {
    fclose(fp);
  }
}