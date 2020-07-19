#include "jni_helper.h"

bool JNIHelper::Jstring2Str(JNIEnv* env, jstring jstr, std::string& str)
{
    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("GB2312");
    jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray barr = (jbyteArray) env->CallObjectMethod(jstr, mid, strencode);
    jsize alen = env->GetArrayLength(barr);
    jbyte* ba = env->GetByteArrayElements(barr, JNI_FALSE);
    if (alen > 0) {
      str.assign((char*)ba, alen);
    }
    env->ReleaseByteArrayElements(barr, ba, 0);
    return true;
}

bool JNIHelper::JbyteArray2Str(JNIEnv* env, jbyteArray &jarray, std::string& str)
{
	jsize alen = env->GetArrayLength(jarray);
	jbyte* ba = env->GetByteArrayElements(jarray, JNI_FALSE);
	if (alen > 0)
	{
		str.assign((char*)ba, alen);
	}
	env->ReleaseByteArrayElements(jarray, ba, 0);

  return true;
}

bool JNIHelper::JbyteArray2Str(JNIEnv* env, jbyteArray& jarray, jint len, std::string& str)
{
  jsize alen = env->GetArrayLength(jarray);
	jbyte* ba = env->GetByteArrayElements(jarray, JNI_FALSE);
	if (alen > 0 && alen >= len)
	{
		str.assign((char*)ba, len);
	}
	env->ReleaseByteArrayElements(jarray, ba, 0);

  return true;
}

bool JNIHelper::AttachJVM(JavaVM* vm, JNIEnv** jniEnv, bool& should_detach)
{
    JNIEnv *env = NULL;
    int env_status = vm->GetEnv((void **)&env, JNI_VERSION_1_6);
    should_detach = false;
    if(env_status == JNI_EDETACHED)
    {
        jint attachResult = vm->AttachCurrentThread(jniEnv, NULL);
        if(attachResult >= 0)
            should_detach = true;
        else
            *jniEnv = NULL;
    }
    else if(JNI_OK == env_status)
    {
        *jniEnv = env;
    }
    else
    {
        *jniEnv = NULL;
    }

    return *jniEnv != NULL;
}

void JNIHelper::DeatchJVM(JavaVM* vm)
{
    vm->DetachCurrentThread();
}
