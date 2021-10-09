//
// Created by cort xu on 2021/10/9.
//

#ifndef MEDIACODEC_NATIVE_PLAY_H
#define MEDIACODEC_NATIVE_PLAY_H
#include "jni_helper.h"

class NativePlay {
public:
    NativePlay();
    ~NativePlay();

public:
    bool Start(ANativeWindow* window, std::string file_path);
    void Update();
    void Stop();

private:
    ANativeWindow*    window_;
    std::atomic_bool  started_;
};


#endif //MEDIACODEC_NATIVE_PLAY_H
