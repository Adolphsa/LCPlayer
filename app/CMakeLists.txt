# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.10.2)

#指定库路径
set(GLM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/3rdparty/libglm)
set(FFMPEG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/3rdparty/libFFmpeg)
set(CPPDIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)

#添加头文件
include_directories(${GLM_DIR}/include)
include_directories(${FFMPEG_DIR}/${ANDROID_ABI}/include)

link_directories(${FFMPEG_DIR}/${ANDROID_ABI}/lib)
set(FFMPEG_LIB avformat avcodec avfilter avutil swresample swscale fdk-aac x264 z c m dl)

set(LIB_SRC ${CPPDIR}/LCPlayer.cpp
        ${CPPDIR}/LCAVCodecHandler.cpp)

set(ANDROID_SYSLIBS android log GLESv3 OpenSLES)

add_library(LCPlayer SHARED ${LIB_SRC})

target_link_libraries(LCPlayer ${FFMPEG_LIB} ${ANDROID_SYSLIBS})