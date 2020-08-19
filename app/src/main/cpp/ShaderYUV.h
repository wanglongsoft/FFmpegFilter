//
// Created by 24909 on 2020/8/5.
//

#ifndef FFMPEGFILTER_SHADERYUV_H
#define FFMPEGFILTER_SHADERYUV_H

#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <math.h>
#include "GlobalContexts.h"
#include "ShaderUtils.h"
#include "LogUtils.h"

class ShaderYUV {
public:
    ShaderYUV(GlobalContexts *global_context);
    ~ShaderYUV();
    GLuint LoadShader(GLenum type, const char *shaderSrc);
    GLuint LoadProgram(const char *vShaderStr, const char *fShaderStr);
    int CreateProgram();
    void changeVideoRotation();
    void Render(uint8_t *data[]);
    void setVideoSize(int width, int height);
    void setWindowSize(int width, int height);
    void initDefMatrix();
    void orthoM(float m[], int mOffset,
                float left, float right, float bottom, float top,
                float near, float far);

    float vertex_coords[12] = {//世界坐标
            -1, -1, 0, // left bottom
            1, -1, 0, // right bottom
            -1, 1, 0,  // left top
            1, 1, 0,   // right top
    };

    float fragment_coords[8] = {//纹理坐标
            0, 1,//left bottom
            1, 1,//right bottom
            0, 0,//left top
            1, 0,//right top
    };

    //后置摄像头
    float fragment_coords_90[8] = {//逆时针90 纹理坐标
            1, 1,
            1, 0,
            0, 1,
            0, 0,
    };

    //前置摄像头
    float fragment_coords_270[8] = {//逆时针270, 上下交换位置 纹理坐标
            0, 1,
            0, 0,
            1, 1,
            1, 0,
    };

    float matrix_scale[16];
    GLint gl_program;
    GLint gl_position;
    GLint gl_textCoord;
    GLint gl_uMatrix;
    GLint gl_video_width;
    GLint gl_video_height;
    GLint gl_window_width;
    GLint gl_window_height;
    GLuint gl_texture_id[3];
    GlobalContexts *context;
    uint8_t *y_data;
    uint8_t *u_data;
    uint8_t *v_data;

    std::string *vertex_shader_graphical = NULL;
    std::string *fragment_shader_graphical = NULL;
};

#endif //FFMPEGFILTER_SHADERYUV_H
