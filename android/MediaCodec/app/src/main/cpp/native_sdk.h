//
// Created by cort xu on 2021/10/9.
//

#ifndef MEDIACODEC_NATIVE_SDK_H
#define MEDIACODEC_NATIVE_SDK_H
#include "jni_helper.h"
#include "native_play.h"

class NativeSdk {
public:
    NativeSdk();
    ~NativeSdk();

public:
    bool PlayStart(ANativeWindow* window, std::string file_path);
    void PlayUpdate();
    void PlayStop();

private:
    NativePlay play_;
};


#endif //MEDIACODEC_NATIVE_SDK_H
