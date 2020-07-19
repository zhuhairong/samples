//
// Created by cort xu on 2021/8/12.
//

#ifndef NATIVECODEC_UTILS_H
#define NATIVECODEC_UTILS_H
#include <android/log.h>
#include <string.h>
#include <string>
#include <time.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>


#define TIMEOUT_US 0

#define SAFE_DELETE(x) { if(x){ delete x; x = NULL;}}
#define SAFE_DELETE_ARRAY(x) { if(x){ delete[] x; x = NULL;}}

#define TAG "[hitest]"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

inline unsigned int GetTickCount() {
  struct timespec on = {0, 0};
  if (clock_gettime(CLOCK_MONOTONIC, &on) != 0) {
    return 0;
  }

  unsigned int tick = on.tv_sec * 1000 + on.tv_nsec / 1000000;
  return tick;
}

inline bool CreatePath(const std::string& path) {
  if (path.empty()) {
    return false;
  }

  std::string full_path = path;

  size_t pos = 0;
  std::string sub_path;
  while (sub_path != full_path) {
    pos = full_path.find_first_of("/", pos + 1);
    if (pos == std::string::npos) {
      sub_path = full_path;
    } else {
      sub_path = full_path.substr(0, pos);
    }

    if (access(sub_path.c_str(), F_OK) == 0) {
      continue;
    }

    if (mkdir(sub_path.c_str(), 0755) != 0) {
      return false;
    }
  }

  return true;
}

inline bool CreateParentPath(const std::string& file_path) {
  size_t pos = file_path.find_last_of("/");
  if (pos == std::string::npos) {
    return false;
  }

  return CreatePath(file_path.substr(0, pos));
}

#endif //NATIVECODEC_UTILS_H
