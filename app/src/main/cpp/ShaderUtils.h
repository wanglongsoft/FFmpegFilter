//
// Created by 24909 on 2020/8/5.
//

#ifndef FFMPEGFILTER_SHADERUTILS_H
#define FFMPEGFILTER_SHADERUTILS_H

#include "LogUtils.h"
#include <cstring>
#include <string>
#include <android/asset_manager.h>

class ShaderUtils {
public:
    static std::string * openAssetsFile(AAssetManager *mgr, char *file_name);
};


#endif //FFMPEGFILTER_SHADERUTILS_H
