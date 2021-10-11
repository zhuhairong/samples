//
// Created by cort xu on 2021/10/9.
//

#ifndef MEDIACODEC_PLAY_CONTEXT_H
#define MEDIACODEC_PLAY_CONTEXT_H
#include "jni_helper.h"

class PlayContext {
public:
    PlayContext();
    ~PlayContext();

public:
    bool Start(JNIEnv* env, jstring jfilepath, jobject jsurface);
    void Update();
    void Stop();

private:
    ANativeWindow*    window_;
    std::atomic_bool  started_;
};


#endif //MEDIACODEC_PLAY_CONTEXT_H
