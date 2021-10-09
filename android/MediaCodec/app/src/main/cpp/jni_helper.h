#ifndef _JNIHelper_h_
#define _JNIHelper_h_

#include <jni.h>
#include <string>
#include <android/native_window_jni.h>

class JNIHelper
{
public:
		static bool Jstring2Str(JNIEnv* env, jstring jstr, std::string& str);
    static bool JbyteArray2Str(JNIEnv* env, jbyteArray& jstr, std::string& str);
		static bool JbyteArray2Str(JNIEnv* env, jbyteArray& jstr, jint len, std::string& str);
    static bool AttachJVM(JavaVM* vm, JNIEnv** env, bool& should_detach);
    static void DeatchJVM(JavaVM* vm);
};

#endif
