//
// Created by 24909 on 2020/8/5.
//

#include "VideoChannel.h"

// 丢包的  函数指针 具体实现 frame
void dropAVFrame(std::queue<AVFrame *> & qq) {
    if (!qq.empty()) {
        AVFrame * frame = qq.front();
        BaseChannel::releaseAVFrame(&frame); // 释放掉
        qq.pop();
    }
}

// 丢包的  函数指针 具体实现 packet
void dropAVPacket(std::queue<AVPacket *> & qq) {
    while (!qq.empty()) {
        AVPacket *avPacket = qq.front();
        // I 帧、B 帧、 P 帧
        //不能丢 I 帧,AV_PKT_FLAG_KEY: I帧（关键帧）
        if (avPacket->flags != AV_PKT_FLAG_KEY) {
            //丢弃非 I 帧
            BaseChannel::releaseAVPacket(&avPacket);
            qq.pop();
        } else {
            break;
        }
    }
}

void* task_video_decode(void * pVoid) {
    VideoChannel * videoChannel = static_cast<VideoChannel *>(pVoid);
    videoChannel->video_decode();
    return 0;
}

void* task_video_filter(void * pVoid) {
    VideoChannel * videoChannel = static_cast<VideoChannel *>(pVoid);
    videoChannel->video_filter();
    return 0;
}

void* task_video_player(void * pVoid) {
    VideoChannel * videoChannel = static_cast<VideoChannel *>(pVoid);
    videoChannel->video_player();
    return 0;
}

VideoChannel::VideoChannel(int streamIndex, AVCodecContext *pContext, GlobalContexts *global_context)
        : BaseChannel(streamIndex, pContext) {
    LOGD("width : %d   height : %d", pContext->width, pContext->height);
    this->packages.setSyncCallback(dropAVPacket);
    this->frames.setSyncCallback(dropAVFrame);
    this->filter_frames.setSyncCallback(dropAVFrame);
    this->filter_frames.setReleaseCallback(releaseAVFrame);
    this->global_context = global_context;
    this->global_context->gl_video_width = pContext->width;
    this->global_context->gl_video_height = pContext->height;
    this->videoFilter = new VideoFilter(pContext, global_context);
    this->video_width = pContext->width;
    this->video_height = pContext->height;
    adjustVideoScaleResolution();
    this->yuv_data[0] = new uint8_t[this->scale_video_width * this->scale_video_height];
    this->yuv_data[1] = new uint8_t[this->scale_video_width * this->scale_video_height >> 2];
    this->yuv_data[2] = new uint8_t[this->scale_video_width * this->scale_video_height >> 2];
}

VideoChannel::~VideoChannel() {
    LOGD("VideoChannel destruct");
    if(NULL != eglDisplayYuv) {
        delete eglDisplayYuv;
        eglDisplayYuv = NULL;
    }
    if(NULL != eglDisplayYuv) {
        delete eglDisplayYuv;
        eglDisplayYuv = NULL;
    }
    if(NULL != videoFilter) {
        delete videoFilter;
        videoFilter = NULL;
    }

    if(NULL != yuv_data[0]) {
        delete yuv_data[0];
        yuv_data[0] = NULL;
    }

    if(NULL != yuv_data[1]) {
        delete yuv_data[1];
        yuv_data[1] = NULL;
    }

    if(NULL != yuv_data[2]) {
        delete yuv_data[2];
        yuv_data[2] = NULL;
    }

    filter_frames.clearQueue();
}

/**
 * 真正的解码，并且，播放
 * 1.解码（解码只有的是原始数据）
 * 2.播放
 */
void VideoChannel::start() {
    isPlaying = 1;
    isStop = 0;
    isPause = 0;

    // 存放未解码的队列开始工作了
    packages.setFlag(1);

    // 存放解码后的队列开始工作了
    frames.setFlag(1);

    // 存放滤镜后的队列开始工作了
    filter_frames.setFlag(1);

    if(!isPause) {
        LOGD("new decode thread and play thread");
        // 1.解码的线程
        pthread_create(&pid_video_decode, 0, task_video_decode, this);

        // 2.滤镜的线程
        pthread_create(&pid_video_filter, 0, task_video_filter, this);

        // 3.播放的线程
        pthread_create(&pid_video_player, 0, task_video_player, this);
    } else {
        LOGD("not new decode thread and play thread");
        isPause = 0;
    }
}

void VideoChannel::stop() {
    isPlaying = 0;
    isPause = 0;
    isStop = 1;
}

/**
 * 运行在异步线程（视频解码函数）
 */
void VideoChannel::video_decode() {
    LOGD("video_decode start");
    AVPacket * packet = 0;
    while (isPlaying || isPause) {
        // 生产快  消费慢
        // 消费速度比生成速度慢（生成100，只消费10个，这样队列会爆）
        // 内存泄漏点2，解决方案：控制队列大小
        if (isPlaying && filter_frames.queueSize() > 100) {
            // 休眠 等待队列中的数据被消费
            av_usleep(10 * 1000);
            continue;
        }

        if(isPause) {
            // 休眠 等待队列中的数据被消费
            av_usleep(10 * 1000);
            continue;
        }

        int ret = packages.pop(packet);

        if (!ret) {
            continue;
        }

        if(isStop) {
            break;
        }

        // 走到这里，就证明，取到了待解码的视频数据包
        ret = avcodec_send_packet(pContext, packet);
        if (ret) {
            if(ret == AVERROR(EAGAIN)) {
                LOGD("avcodec_send_packet fail : EAGAIN");
            } else if(ret == AVERROR_EOF) {
                LOGD("avcodec_send_packet fail : AVERROR_EOF");
            } else if(ret == AVERROR(EINVAL)) {
                LOGD("avcodec_send_packet fail : EINVAL");
            } else if(ret == AVERROR(ENOMEM)) {
                LOGD("avcodec_send_packet fail : ENOMEM");
            } else {
                LOGD("avcodec_send_packet fail : Other");
            }
            // 失败了（给解码器上下文发送Packet失败）
            LOGD("avcodec_send_packet fail : %d", ret);
            break;
        }

        // 走到这里，就证明，packet，用完毕了，
        // 成功了（给“解码器上下文”发送Packet成功），那么就可以释放packet了
        releaseAVPacket(&packet);

        AVFrame *frame = av_frame_alloc(); // AVFrame 拿到解码后的原始数据包
        ret = avcodec_receive_frame(pContext, frame);
        if (ret == AVERROR(EAGAIN)) {
            // 未取到关键帧，重来，重新取
            LOGD("not key frame , continue");
            releaseAVFrame(&frame);
            continue;
        } else if(ret != 0) {//decode error
            releaseAVFrame(&frame); // 内存释放点
            LOGD("avcodec_receive_frame fail");
            break;
        }

        // 终于取到了，解码后的视频数据（原始数据）
        frames.push(frame); // 加入队列
    }
    releaseAVPacket(&packet);
    LOGD("VideoChannel video_decode end");
}

/**
 * 运行在异步线程（视频滤镜函数）
 */
void VideoChannel::video_filter() {
    LOGD("VideoChannel video_filter start");
    AVFrame * frame = NULL;
    AVFrame * filter_frame = NULL;
    while(isPlaying || isPause) {
        if (isPause) {
            av_usleep(20 * 1000);
            continue;
        }

        if (isPlaying && frames.queueSize() > 100) {
            // 休眠 等待队列中的数据被消费
            av_usleep(10 * 1000);
            continue;
        }

        if (isStop) {
            break;
        }

        int ret = frames.pop(frame);

        if (!ret) {
            continue;
        }

        filter_frame = av_frame_alloc();
        videoFilter->filter_video(frame, filter_frame, filter_frames);
        releaseAVFrame(&frame);
    }
    LOGD("VideoChannel video_filter end");
}

/**
 * 运行在异步线程（视频渲染函数）
 */
void VideoChannel::video_player() {
    LOGD("VideoChannel video_player start");
    AVFrame * frame = NULL;

    double video_delay = 0;

    if(NULL == eglDisplayYuv) {
        eglDisplayYuv = new EGLDisplayYUV(this->global_context->nativeWindow, this->global_context);
    }

    SwsContext *sws_ctx = sws_getContext(
            this->video_width, this->video_height, pContext->pix_fmt,
            this->scale_video_width, this->scale_video_height, AV_PIX_FMT_YUV420P,
            SWS_BILINEAR, NULL, NULL, NULL
    );

    if(sws_ctx == NULL) {
        LOGD("sws_getContext fail, return");
        return;
    }
    LOGD("sws_getContext init success");

    uint8_t *local_yuv_data[4];
    int yuv_linesizes[4];
    int alloc_ret = av_image_alloc(local_yuv_data, yuv_linesizes, scale_video_width,
            scale_video_height, AV_PIX_FMT_YUV420P, 1);

    if(alloc_ret < 0) {
        LOGD("av_image_alloc fail, return");
        return;
    }

    LOGD("yuv_linesizes: %d, %d, %d, %d", yuv_linesizes[0], yuv_linesizes[1],
            yuv_linesizes[2], yuv_linesizes[3]);

    if ((this->global_context->eglSurface != NULL)
        || (this->global_context->eglContext != NULL)
        || (this->global_context->eglDisplay != NULL)) {
        eglDisplayYuv->eglClose();
    }
    eglDisplayYuv->eglOpen();

    if(NULL == shaderYuv) {
        this->global_context->gl_video_width = scale_video_width;
        this->global_context->gl_video_height = scale_video_height;
        shaderYuv = new ShaderYUV(this->global_context);
    }

    shaderYuv->CreateProgram();

    double delay_time_per_frame = 1.0 / av_q2d(this->avg_frame_rate);

    LOGD("delay_time_per_frame: %lf", delay_time_per_frame);
    while(isPlaying || isPause) {

        if(isPause) {
            av_usleep(20 * 1000);
            continue;
        }

        int ret = filter_frames.pop(frame);

        // 如果停止播放，跳出循环, 出了循环，就要释放

        if (!ret) {
            continue;
        }

        if (isStop) {
            break;
        }

        if((this->scale_video_width == this->video_width)
           && (this->scale_video_height == this->video_height)) {//无需调整帧数据宽高
            avFrameDataToYUVData(frame, yuv_data);
        } else {//需调整帧数据宽高
            sws_scale(sws_ctx, frame->data,
                      frame->linesize, 0, this->video_height,
                      local_yuv_data, yuv_linesizes);

            avFrameDataToYUVDataScale(local_yuv_data, this->scale_video_width,
                    this->scale_video_height, this->yuv_data, yuv_linesizes);
        }

        //每一帧还有自己的额外延时时间
        //extra_delay = repeat_pict / (2*fps)
        double extra_delay = frame->repeat_pict / (2 * av_q2d(this->avg_frame_rate));
        double real_delay = delay_time_per_frame + extra_delay;
        if(*clock > 0) { //简易音视频同步
            video_delay = frame->best_effort_timestamp * av_q2d(time_base) - *clock;
            if(video_delay > 0) {//视频比音频快，延时渲染
                if(video_delay > 1) {
                    av_usleep(real_delay * 1000000 * 2);
                } else {
                    av_usleep((real_delay + video_delay) * 1000000);
                }
            } else {//音频比视频快，直接渲染视频帧
                //不延时，加快播放，不要丢帧，否则画面跳变
            }
        } else {//无音频时钟参考，按照默认帧率播放
            //没有音频,类似GIF
            av_usleep(real_delay * 1000000);
        }

        // OpenGL渲染YUV数据
        shaderYuv->Render(this->yuv_data);
        releaseAVFrame(&frame); // 渲染完了，frame没用了，释放掉
    }
    releaseAVFrame(&frame);
    isPlaying = 0;
    sws_freeContext(sws_ctx);
    av_freep(&yuv_data[0]);
}

void VideoChannel::setRenderCallback(RenderCallback renderCallback) {
    this->renderCallback = renderCallback;
}

void VideoChannel::setTimeBase(AVRational base) {
    this->time_base = base;
    this->global_context->video_timebase_num = base.num;
    this->global_context->video_timebase_den = base.den;
    this->videoFilter->initFilters();
}

void VideoChannel::setAvgFrameRate(AVRational rate) {
    this->avg_frame_rate = rate;
}

void VideoChannel::setReferenceClock(double *value) {
    clock = value;
}

void VideoChannel::pause() {
    isPlaying = 0;
    isStop = 0;
    isPause = 1;
}

//复制YUV数据到新内存，原因：AVFrame的linesize可能不等于帧的宽度，
// 即yuv数据可能不连续
void VideoChannel::avFrameDataToYUVData(AVFrame *frame, uint8_t **dst_data) {
    uint32_t pitchY = frame->width;
    uint32_t pitchU = frame->width >> 1;
    uint32_t pitchV = frame->width >> 1;
    uint8_t* avY = dst_data[0];
    uint8_t* avU = dst_data[1];
    uint8_t* avV = dst_data[2];

    for (int y_index = 0; y_index < frame->height; ++y_index) {
        memcpy(avY, frame->data[0] + y_index * frame->linesize[0], pitchY);
        avY += pitchY;
    }

    for (int u_index = 0; u_index < frame->height >> 1; ++u_index) {
        memcpy(avU, frame->data[1] + u_index * frame->linesize[1], pitchU);
        avU += pitchU;
    }

    for (int v_index = 0; v_index < frame->height >> 1; ++v_index) {
        memcpy(avV, frame->data[2] + v_index * frame->linesize[2], pitchV);
        avV += pitchV;
    }
}

//复制YUV数据到新内存，原因：AVFrame的linesize可能不等于帧的宽度，
// 即yuv数据可能不连续
void VideoChannel::avFrameDataToYUVDataScale(uint8_t **src_data, int src_width, int src_height,
                                             uint8_t **dst_data, int *line_size) {
    uint32_t pitchY = src_width;
    uint32_t pitchU = src_width >> 1;
    uint32_t pitchV = src_width >> 1;
    uint8_t* avY = dst_data[0];
    uint8_t* avU = dst_data[1];
    uint8_t* avV = dst_data[2];

    for (int y_index = 0; y_index < src_height; ++y_index) {
        memcpy(avY, src_data[0] + y_index * line_size[0], pitchY);
        avY += pitchY;
    }

    for (int u_index = 0; u_index < src_height >> 1; ++u_index) {
        memcpy(avU, src_data[1] + u_index * line_size[1], pitchU);
        avU += pitchU;
    }

    for (int v_index = 0; v_index < src_height >> 1; ++v_index) {
        memcpy(avV, src_data[2] + v_index * line_size[2], pitchV);
        avV += pitchV;
    }
}

void VideoChannel::adjustVideoScaleResolution() {
    if(this->video_width % 8 != 0) {
        this->scale_video_width = ((this->video_width / 8) + 1) * 8;
    } else {
        this->scale_video_width = this->video_width;
    }
    if(this->video_height % 2 != 0) {
        this->scale_video_height = ((this->video_height / 2) + 1) * 2;
    } else {
        this->scale_video_height = this->video_height;
    }
}
