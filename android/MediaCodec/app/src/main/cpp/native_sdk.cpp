//
// Created by cort xu on 2021/10/9.
//

#include "native_sdk.h"

NativeSdk::NativeSdk() {
}

NativeSdk::~NativeSdk() {
}

bool NativeSdk::PlayStart(ANativeWindow* window, std::string file_path) {
  return play_.Start(window, file_path);
}

void NativeSdk::PlayUpdate() {
  play_.Update();
}

void NativeSdk::PlayStop() {
  play_.Stop();
}