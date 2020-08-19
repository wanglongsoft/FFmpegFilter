//
// Created by 24909 on 2020/8/6.
//

#ifndef FFMPEGFILTER_VIDEOFILTER_H
#define FFMPEGFILTER_VIDEOFILTER_H

#include "LogUtils.h"
#include "GlobalContexts.h"
#include "SafeQueue.h"

extern "C" {
    #include <libavfilter/avfilter.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/opt.h>
    #include <libavfilter/buffersrc.h>
    #include <libavfilter/buffersink.h>
}

class VideoFilter {
public:
    VideoFilter(AVCodecContext *pContext, GlobalContexts *global_context);
    ~VideoFilter();

    void initFilters();
    int filter_video(AVFrame *frame, AVFrame *filt_frame, SafeQueue<AVFrame *> &queue);
    //yuv420p数据写入文件
    int yuv_data_save(unsigned char *data[], int width, int height, FILE *fd);

//    const char *filter_desc = "delogo=x=20:y=30:w=210:h=100:show=1";
    const char *filter_desc = "drawbox=x=30:y=100:w=200:h=100:c=red";
    AVFilterInOut *inputs = NULL;
    AVFilterInOut *outputs = NULL;
    AVFilterContext *buf_ctx = NULL;
    AVFilterContext *buf_sink_ctx = NULL;
    AVFilterGraph *graph = NULL;
    const AVFilter *buffer_filter = NULL;
    const AVFilter *buffer_sink_filter = NULL;
    char filter_args[512];
    int ret;
    AVCodecContext *pContext = NULL;
    GlobalContexts *global_context = NULL;
//    FILE *dst_fd = NULL;
//    FILE *src_fd = NULL;
//    const char *src_file_name = "/storage/emulated/0/filefilm/src_video_filter.yuv";
//    const char *dst_file_name = "/storage/emulated/0/filefilm/dst_video_filter.yuv";
    enum AVPixelFormat pix_fmts[3] = {AV_PIX_FMT_YUV420P, AV_PIX_FMT_GRAY8, AV_PIX_FMT_NONE};
};


#endif //FFMPEGFILTER_VIDEOFILTER_H
