#include <jni.h>
#include <string>

#include "LCPlayer.h"
#include "LCNDKLogDef.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

#ifdef __cplusplus
extern "C" {
#endif


jstring Java_com_lc_lp_LCPlayer_stringFromJNI(JNIEnv *env, jobject thiz) {
    std::string hello = "Hello from C++ LCPlayer";
    hello += avcodec_configuration();
    LOGD("ffmpeg config --- %s", hello.c_str());
    return env->NewStringUTF(hello.c_str());
}

void Java_com_lc_lp_LCPlayer_ndkInitGL(JNIEnv *env, jobject thiz, jobject asset_manager) {
    LOGD("ndkInitGL");
}

void Java_com_lc_lp_LCPlayer_ndkResizeGL(JNIEnv *env, jobject thiz, jint width, jint height) {
    LOGD("ndkResizeGL");
}

void Java_com_lc_lp_LCPlayer_ndkPaintGL(JNIEnv *env, jobject thiz) {
    LOGD("ndkPaintGL");
}

#ifdef __cplusplus
}
#endif