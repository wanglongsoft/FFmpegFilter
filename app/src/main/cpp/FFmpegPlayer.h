//
// Created by 24909 on 2020/8/5.
//

#ifndef FFMPEGFILTER_FFMPEGPLAYER_H
#define FFMPEGFILTER_FFMPEGPLAYER_H

#include <sys/types.h>
#include "AVCallback.h"
#include "AudioChannel.h"
#include "VideoChannel.h"
#include "LogUtils.h"
#include "GlobalContexts.h"
#include <pthread.h>

extern "C" {
    #include "libavformat/avformat.h"
};


class FFmpegPlayer {
public:
    FFmpegPlayer();

    FFmpegPlayer(const char *data_source, AVCallback *pCallback, GlobalContexts *global_context);

    ~FFmpegPlayer(); // 这个类没有子类，所以没有添加虚函数

    void prepare();

    void prepare_();

    void start();

    void start_();

    void setRenderCallback(RenderCallback callback);

    void pause();

    void stop();

private:
    char *data_source = NULL;

    pthread_t pid_prepare ;

    AVFormatContext *formatContext = NULL;

    AudioChannel *audioChannel = NULL;
    VideoChannel *videoChannel = NULL;
    AVCallback *pCallback = NULL;
    RenderCallback renderCallback;
    pthread_t pid_start;
    bool isPlaying;
    bool isPause;
    bool isStop;
    double referenceClock;
    GlobalContexts *global_context = NULL;
};

#endif //FFMPEGFILTER_FFMPEGPLAYER_H
