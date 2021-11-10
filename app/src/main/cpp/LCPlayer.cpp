#include <jni.h>
#include <string>

#include "LCPlayer.h"
#include "LCNDKCommonDef.h"
#include "LCAVCodecHandler.h"

using namespace std;

LCAVCodecHandler m_avCodecHandler;

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

void Java_com_lc_lp_LCPlayer_ndkInitVideoPlayer(JNIEnv *env, jobject thiz) {

}

void Java_com_lc_lp_LCPlayer_ndkStartPlayerWithFile(JNIEnv *env, jobject thiz, jstring file_name) {
    string filePath = env->GetStringUTFChars(file_name, 0);
    LOGD("video path = %s", filePath.c_str());

    m_avCodecHandler.StopPlayVideo();
    m_avCodecHandler.SetVideoFilePath(filePath);
    m_avCodecHandler.InitVideoCodec();
//    m_avCodecHandler.StartPlayVideo();
}

void Java_com_lc_lp_LCPlayer_ndkPauseVideoPlay(JNIEnv *env, jobject thiz) {

}

void Java_com_lc_lp_LCPlayer_ndkStopVideoPlayer(JNIEnv *env, jobject thiz) {

}

jfloat Java_com_lc_lp_LCPlayer_ndkGetVideoSizeRatio(JNIEnv *env, jobject thiz) {

}

jfloat Java_com_lc_lp_LCPlayer_ndkGetVideoTotalSeconds(JNIEnv *env, jobject thiz) {

}

void Java_com_lc_lp_LCPlayer_ndkSeekMedia(JNIEnv *env, jobject thiz, jfloat n_pos) {

}

#ifdef __cplusplus
}
#endif