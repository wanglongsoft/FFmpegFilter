//
// Created by 24909 on 2020/8/5.
//

#ifndef FFMPEGFILTER_AVCALLBACK_H
#define FFMPEGFILTER_AVCALLBACK_H

#include <jni.h>

#include "LogUtils.h"
#include "CommonDefine.h"

class AVCallback {
public:
    AVCallback(JavaVM *pVm, JNIEnv *pEnv, jobject instance);
    ~AVCallback();
    void onError(int error_code, int thread_mode);
    void onPrepare(int thread_mode);
private:
    JavaVM * jvm;
    JNIEnv * jenv;
    jobject instance;
    jmethodID methodId_onError;
    jmethodID methodId_onPrepare;
};


#endif //FFMPEGFILTER_AVCALLBACK_H
