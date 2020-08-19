# FFmpegFilter
基于Android平台的FFmpeg滤镜简单教程,该工程以drawbox示例，展示了代码调用FFmpeg滤镜的基本流程
#### 编译环境
FFmpeg版本： **4.2.2**  
NDK版本：**r17c**
#### 运行环境
* x86(模拟器)
* arm64-v8a(64位手机)
#### 功能点
* 使用OpenGLES3.0渲染视频，利用GPU进行YUV转RGB显示
* 使用FFmpeg的Scale模块缩放视频
* 使用drawbox(矩形框)作为滤镜的简单演示
#### 运行效果(工程根目录images文件夹)
![渲染效果](https://github.com/wanglongsoft/FFmpegFilter/tree/master/image/play.jpg)