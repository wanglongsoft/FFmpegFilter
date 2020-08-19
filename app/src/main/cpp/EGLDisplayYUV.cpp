//
// Created by 24909 on 2020/8/5.
//

#include "EGLDisplayYUV.h"

EGLDisplayYUV::EGLDisplayYUV(ANativeWindow *window, GlobalContexts *context) {
    this->nativeWindow = window;
    this->global_context = context;
}

EGLDisplayYUV::~EGLDisplayYUV() {

}

int EGLDisplayYUV::eglOpen() {
    EGLDisplay eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (eglDisplay == EGL_NO_DISPLAY ) {
        LOGD("eglGetDisplay failure : %d", eglGetError());
        return -1;
    }
    LOGD("eglGetDisplay ok");
    this->global_context->eglDisplay = eglDisplay;

    EGLint majorVersion;//主版本号
    EGLint minorVersion;//次版本号
    EGLBoolean success = eglInitialize(eglDisplay, &majorVersion,
                                       &minorVersion);
    if (!success) {
        LOGD("eglInitialize failure: %d", eglGetError());
        return -1;
    }
    LOGD("eglInitialize ok");

    EGLint numConfigs;
    EGLConfig config;
    const EGLint CONFIG_ATTRIBS[] = { EGL_RED_SIZE, 8,
                                      EGL_GREEN_SIZE, 8,
                                      EGL_BLUE_SIZE, 8,
                                      EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                                      EGL_NONE // the end
    };

    success = eglChooseConfig(eglDisplay, CONFIG_ATTRIBS, &config, 1, &numConfigs);
    if (!success) {
        LOGD("eglChooseConfig failure: %d", eglGetError());
        return -1;
    }
    LOGD("eglChooseConfig ok");

    EGLSurface eglSurface = eglCreateWindowSurface(eglDisplay, config,
                                                   this->nativeWindow, 0);
    if (EGL_NO_SURFACE == eglSurface) {
        LOGD("eglCreateWindowSurface failure: %d", eglGetError());
        return -1;
    }

    LOGD("eglCreateWindowSurface ok");
    this->global_context->eglSurface = eglSurface;

    const EGLint attribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
    EGLContext elgContext = eglCreateContext(eglDisplay, config, EGL_NO_CONTEXT,
                                             attribs);
    if (elgContext == EGL_NO_CONTEXT ) {
        LOGD("eglCreateContext failure, error is %d", eglGetError());
        return -1;
    }
    LOGD("eglCreateContext ok");
    this->global_context->eglContext = elgContext;
    return 0;
}

int EGLDisplayYUV::eglClose() {
    EGLBoolean success = eglDestroySurface(this->global_context->eglDisplay, this->global_context->eglSurface);
    if (!success) {
        LOGD("eglDestroySurface failure.");
    }

    success = eglDestroyContext(this->global_context->eglDisplay, this->global_context->eglContext);
    if (!success) {
        LOGD("eglDestroyContext failure.");
    }

    success = eglTerminate(this->global_context->eglDisplay);
    if (!success) {
        LOGD("eglTerminate failure.");
    }

    this->global_context->eglSurface = NULL;
    this->global_context->eglContext = NULL;
    this->global_context->eglDisplay = NULL;
    return 0;
}