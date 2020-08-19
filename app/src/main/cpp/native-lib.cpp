#include <jni.h>
#include <string>
#include <pthread.h>
#include <android/native_window_jni.h> // 是为了 渲染到屏幕支持的
#include <android/asset_manager_jni.h>

#include "LogUtils.h"
#include "FFmpegPlayer.h"
#include "GlobalContexts.h"

FFmpegPlayer *player = NULL;
ANativeWindow * nativeWindow = NULL;
GlobalContexts *global_context = NULL;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

extern "C"
JNIEXPORT void JNICALL
Java_com_soft_function_FunctionControl_setSurface(JNIEnv *env, jobject thiz, jobject surface) {
    LOGD("setSurface in");
    pthread_mutex_lock(&mutex);

    if (nativeWindow) {
        ANativeWindow_release(nativeWindow);
        nativeWindow = 0;
    }

    // 创建新的窗口用于视频显示
    nativeWindow = ANativeWindow_fromSurface(env, surface);
    if(NULL == global_context) {
        global_context = new GlobalContexts();
    }
    global_context->nativeWindow = nativeWindow;
    pthread_mutex_unlock(&mutex);
    LOGD("setSurface out");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_soft_function_FunctionControl_setSurfaceSize(JNIEnv *env, jobject thiz, jint width,
                                                      jint height) {
    LOGD("setSurfaceSize in");
    pthread_mutex_lock(&mutex);
    if(NULL == global_context) {
        global_context = new GlobalContexts();
    }
    global_context->gl_window_width = width;
    global_context->gl_window_height = height;
    pthread_mutex_unlock(&mutex);
    LOGD("setSurfaceSize out");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_soft_function_FunctionControl_saveAssetManager(JNIEnv *env, jobject thiz,
                                                        jobject manager) {
    LOGD("saveAssetManager in");
    pthread_mutex_lock(&mutex);
    AAssetManager *mgr = AAssetManager_fromJava(env, manager);
    if(NULL == global_context) {
        global_context = new GlobalContexts();
    }
    global_context->assetManager = mgr;
    pthread_mutex_unlock(&mutex);
    LOGD("saveAssetManager out");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_soft_function_FunctionControl_releaseResources(JNIEnv *env, jobject thiz) {
    LOGD("releaseResources in");
    pthread_mutex_lock(&mutex);
    if(NULL != player) {
        delete player;
        player = NULL;
    }
    if(NULL != global_context) {
        delete global_context;
        global_context = NULL;
    }
    pthread_mutex_unlock(&mutex);
    LOGD("releaseResources out");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_soft_function_FunctionControl_preparePlayer(JNIEnv *env, jobject thiz,
                                                     jstring data_source) {
    LOGD("preparePlayer in");
    pthread_mutex_lock(&mutex);
    if(NULL == global_context) {
        global_context = new GlobalContexts();
    }

    if(NULL != player) {
        delete player;
        player = NULL;
    }

    const char * data = env->GetStringUTFChars(data_source, NULL);
    player = new FFmpegPlayer(data, NULL, global_context);
    player->setRenderCallback(NULL);
    player->prepare();
    env->ReleaseStringUTFChars(data_source, data);
    pthread_mutex_unlock(&mutex);
    LOGD("preparePlayer out");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_soft_function_FunctionControl_startPlayer(JNIEnv *env, jobject thiz) {
    LOGD("startPlayer in");
    pthread_mutex_lock(&mutex);
    if(NULL != player) {
        player->start();
    }
    pthread_mutex_unlock(&mutex);
    LOGD("startPlayer out");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_soft_function_FunctionControl_pausePlayer(JNIEnv *env, jobject thiz) {
    LOGD("pausePlayer in");
    pthread_mutex_lock(&mutex);
    if(NULL != player) {
        player->pause();
    }
    pthread_mutex_unlock(&mutex);
    LOGD("pausePlayer out");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_soft_function_FunctionControl_stopPlayer(JNIEnv *env, jobject thiz) {
    LOGD("stopPlayer in");
    pthread_mutex_lock(&mutex);
    if(NULL != player) {
        player->stop();
    }
    pthread_mutex_unlock(&mutex);
    LOGD("stopPlayer out");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_soft_function_FunctionControl_releasePlayer(JNIEnv *env, jobject thiz) {
    LOGD("releasePlayer in");
    pthread_mutex_lock(&mutex);
    if(NULL != player) {
        delete player;
        player = NULL;
    }
    pthread_mutex_unlock(&mutex);
    LOGD("releasePlayer out");
}