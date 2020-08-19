//
// Created by 24909 on 2020/8/6.
//

#include "VideoFilter.h"

VideoFilter::VideoFilter(AVCodecContext *pContext, GlobalContexts *global_context) {
    this->pContext = pContext;
    this->global_context = global_context;
}

VideoFilter::~VideoFilter() {
//    fflush(this->src_fd);
//    fclose(this->src_fd);
//    this->src_fd = NULL;
//
//    fflush(this->dst_fd);
//    fclose(this->dst_fd);
//    this->dst_fd = NULL;
}

void VideoFilter::initFilters() {
    LOGD("initFilters in");
    this->inputs = avfilter_inout_alloc();
    this->outputs = avfilter_inout_alloc();
    if((NULL == this->inputs)
        || (NULL == this->outputs)) {
        LOGD("avfilter_inout_alloc fail, return");
        return;
    }

    this->graph = avfilter_graph_alloc();
    if((NULL == this->graph)
        || (NULL == this->graph)) {
        LOGD("avfilter_graph_alloc fail, return");
        return;
    }

    this->buffer_filter = avfilter_get_by_name("buffer");
    this->buffer_sink_filter = avfilter_get_by_name("buffersink");
    if((NULL == this->buffer_filter)
       || (NULL == this->buffer_sink_filter)) {
        LOGD("avfilter_get_by_name fail, return");
        return;
    }

    snprintf(filter_args, 512,
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
             this->pContext->width, this->pContext->height,
             this->pContext->pix_fmt,
             this->global_context->video_timebase_num, this->global_context->video_timebase_den,
             this->pContext->sample_aspect_ratio.num, this->pContext->sample_aspect_ratio.den);


    LOGD("initFilters filter_args : %s", this->filter_args);

    ret = avfilter_graph_create_filter(&this->buf_ctx, this->buffer_filter, "in",
            this->filter_args, NULL, this->graph);
    if(ret < 0) {
        LOGD("avfilter_graph_create_filter buf_ctx fail：%d, return", ret);
        return;
    }

    ret = avfilter_graph_create_filter(&this->buf_sink_ctx, this->buffer_sink_filter, "out",
            NULL, NULL, this->graph);
    if(ret < 0) {
        LOGD("avfilter_graph_create_filter buf_sink_ctx fail：%d, return", ret);
        return;
    }

    av_opt_set_int_list(this->buf_sink_ctx, "pix_fmts", this->pix_fmts,
            AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);

    inputs->name = av_strdup("out");
    inputs->filter_ctx = this->buf_sink_ctx;
    inputs->pad_idx = 0;
    inputs->next = NULL;

    outputs->name = av_strdup("in");
    outputs->filter_ctx = this->buf_ctx;
    outputs->pad_idx = 0;
    outputs->next = NULL;

    ret = avfilter_graph_parse_ptr(this->graph, filter_desc, &inputs, &outputs, NULL);
    if(ret < 0) {
        LOGD("avfilter_graph_parse_ptr fail：%d, return", ret);
        return;
    }

    ret = avfilter_graph_config(this->graph, NULL);
    if(ret < 0) {
        LOGD("avfilter_graph_config fail：%d, return", ret);
        return;
    }

//    this->dst_fd = fopen(this->dst_file_name, "wb");
//    if(!this->dst_fd) {
//        LOGD("fopen file : %s fail", this->dst_file_name);
//        return;
//    }
//
//    this->src_fd = fopen(this->src_file_name, "wb");
//    if(!this->src_fd) {
//        LOGD("fopen file : %s fail", this->src_file_name);
//        return;
//    }

    avfilter_inout_free(&this->inputs);
    this->inputs = NULL;
    avfilter_inout_free(&this->outputs);
    this->outputs = NULL;

    LOGD("initFilters out");
}

int VideoFilter::filter_video(AVFrame *frame, AVFrame *filt_frame, SafeQueue<AVFrame *> &queue) {
    int ret = -1;
//    yuv_data_save(frame->data, frame->width, frame->height, this->src_fd);
    if ((ret = av_buffersrc_add_frame(this->buf_ctx, frame)) < 0 ) {
        LOGD("Failed to feed to filter graph!");
        return ret;
    }

    while(1) {
        ret = av_buffersink_get_frame(buf_sink_ctx, filt_frame);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        }

        if(ret < 0) {
            return ret;
        }
        queue.push(filt_frame);
//        yuv_data_save(filt_frame->data, filt_frame->width, filt_frame->height, this->dst_fd);
    }
    return ret;
}

int VideoFilter::yuv_data_save(unsigned char **data, int width,
        int height, FILE *fd) {
    uint32_t pitchY = width;
    uint32_t pitchU = width >> 1;
    uint32_t pitchV = width >> 1;
    uint8_t* avY = data[0];
    uint8_t* avU = data[1];
    uint8_t* avV = data[2];

    //YUV数据之Y
    for (int i = 0; i < height; i++) {
        fwrite(avY, 1, pitchY, fd);
        avY += pitchY;
    }

    //YUV数据之U，！！！！  height / 2
    for (int i = 0; i < height / 2; i++) {
        fwrite(avU, 1, pitchU, fd);
        avU += pitchU;
    }

    //YUV数据之V，！！！！  height / 2
    for (int i = 0; i < height / 2; i++) {
        fwrite(avV, 1, pitchV, fd);
        avV += pitchV;
    }
    return 0;
}
