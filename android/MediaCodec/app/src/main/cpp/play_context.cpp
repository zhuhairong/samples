//
// Created by cort xu on 2021/10/9.
//

#include "play_context.h"

PlayContext::PlayContext() : window_(nullptr), started_(false) {
}

PlayContext::~PlayContext() {
  Stop();
}

bool PlayContext::Start(JNIEnv* env, jstring jfilepath, jobject jsurface) {
  if (!started_) {
    return false;
  }

  window_ = window;
  started_ = true;
  return true;
}

void PlayContext::Update() {
  if (!started_) {
    return;
  }
}

void PlayContext::Stop() {
  if (window_) {
    ANativeWindow_release(window_);
    window_ = nullptr;
  }

  started_ = false;
}