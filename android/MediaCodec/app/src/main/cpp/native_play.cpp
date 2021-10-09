//
// Created by cort xu on 2021/10/9.
//

#include "native_play.h"

NativePlay::NativePlay() : window_(nullptr), started_(false) {
}

NativePlay::~NativePlay() {
  Stop();
}

bool NativePlay::Start(ANativeWindow* window, std::string file_path) {
  if (!started_) {
    return false;
  }

  window_ = window;
  started_ = true;
  return true;
}

void NativePlay::Update() {
  if (!started_) {
    return;
  }
}

void NativePlay::Stop() {
  if (window_) {
    ANativeWindow_release(window_);
    window_ = nullptr;
  }

  started_ = false;
}