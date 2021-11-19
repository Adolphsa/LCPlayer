#include <jni.h>
#include <string>

#include "LCPlayer.h"
#include "LCNDKCommonDef.h"
#include "LCAVCodecHandler.h"
#include "LCGLVideoRender.h"

using namespace std;

LCAVCodecHandler m_avCodecHandler;
LCGLVideoRender m_glVideoRender;

JavaVM *g_jvm = NULL;
jobject g_obj = NULL;

#ifdef __cplusplus
extern "C" {
#endif

void updateVideoData(YUVData_Frame* yuvFrame, unsigned long userData);
void updateVideoPts(float pts, unsigned long userData);

jstring Java_com_lc_lp_LCPlayer_stringFromJNI(JNIEnv *env, jobject thiz) {
    std::string hello = "Hello from C++ LCPlayer";
    hello += avcodec_configuration();
    LOGD("ffmpeg config --- %s", hello.c_str());
    return env->NewStringUTF(hello.c_str());
}

void Java_com_lc_lp_LCPlayer_ndkInitGL(JNIEnv *env, jobject thiz, jobject asset_manager) {
    LOGD("ndkInitGL");
    AAssetManager *aAssetManager = AAssetManager_fromJava(env, asset_manager);
    if (NULL != aAssetManager) {
        m_glVideoRender.SetupAssetManager(aAssetManager);
    }
    m_glVideoRender.InitGL();
}

void Java_com_lc_lp_LCPlayer_ndkResizeGL(JNIEnv *env, jobject thiz, jint width, jint height) {
    LOGD("ndkResizeGL");
    m_glVideoRender.ResizeGL(width, height);
}

void Java_com_lc_lp_LCPlayer_ndkPaintGL(JNIEnv *env, jobject thiz) {
//    LOGD("ndkPaintGL");
    m_glVideoRender.PaintGL();
}

void Java_com_lc_lp_LCPlayer_ndkInitVideoPlayer(JNIEnv *env, jobject thiz)
{
    m_avCodecHandler.SetupUpdateVideoCallback(updateVideoData, NULL);
    m_avCodecHandler.SetupUpdateCurrentPTSCallback(updateVideoPts, NULL);
    env->GetJavaVM(&g_jvm);
    g_obj = env->NewGlobalRef(thiz);
}

void Java_com_lc_lp_LCPlayer_ndkStartPlayerWithFile(JNIEnv *env, jobject thiz, jstring file_name)
{
    string filePath = env->GetStringUTFChars(file_name, 0);
    LOGD("video path = %s", filePath.c_str());

    m_avCodecHandler.StopPlayVideo();
    m_avCodecHandler.SetVideoFilePath(filePath);
    m_avCodecHandler.InitVideoCodec();
    m_avCodecHandler.StartPlayVideo();
}

void Java_com_lc_lp_LCPlayer_ndkPauseVideoPlay(JNIEnv *env, jobject thiz)
{
    m_avCodecHandler.SetMediaStatusPause();
}

void Java_com_lc_lp_LCPlayer_ndkPlayVideoPlay(JNIEnv *env, jobject thiz) {
    m_avCodecHandler.SetMediaStatusPlay();
}

void Java_com_lc_lp_LCPlayer_ndkStopVideoPlayer(JNIEnv *env, jobject thiz)
{
    m_avCodecHandler.StopPlayVideo();
}

jint Java_com_lc_lp_LCPlayer_ndkGetPlayStatus(JNIEnv *env, jobject thiz) {
    return m_avCodecHandler.GetPlayerStatus();
}

jfloat Java_com_lc_lp_LCPlayer_ndkGetVideoSizeRatio(JNIEnv *env, jobject thiz)
{
    int vWidth = m_avCodecHandler.GetVideoWidth();
    int vHeight = m_avCodecHandler.GetVideoHeight();
    jfloat  ratio = (jfloat)vWidth / (jfloat)vHeight;
    LOGD("VIDEO SIZE RATIO: %f",ratio);
    return ratio;
}

jfloat Java_com_lc_lp_LCPlayer_ndkGetVideoTotalSeconds(JNIEnv *env, jobject thiz)
{
    jfloat totalSeconds = m_avCodecHandler.GetMediaTotalSeconds();
    return totalSeconds;
}

void Java_com_lc_lp_LCPlayer_ndkSeekMedia(JNIEnv *env, jobject thiz, jfloat n_pos)
{
    m_avCodecHandler.SeekMedia(n_pos);
}

void updateVideoData(YUVData_Frame* yuvFrame, unsigned long userData)
{
    if (yuvFrame == NULL) {
        return;
    }

    m_glVideoRender.RendVideo(yuvFrame);

//    LOGD("UPDATE VIDEO DATA: %d %d",yuvFrame->width,yuvFrame->height);

    JNIEnv *m_env;
    jmethodID m_methodId;
    jclass m_class;

    //Attach主线程
    if (g_jvm->AttachCurrentThread(&m_env, NULL) != JNI_OK) {
        LOGE("JVM: AttachCurrentThread failed");
        return ;
    }

    //找到对应的类
    m_class = m_env->GetObjectClass(g_obj);
    if (m_class == NULL) {
        LOGD("JVM: FindClass error.....");
        if(g_jvm->DetachCurrentThread() != JNI_OK)
        {
            LOGE("JVM: DetachCurrentThread failed");
        }
        return ;
    }

    //再获得类中的方法
    m_methodId = m_env->GetMethodID(m_class, "OnVideoRenderCallback", "()V");
    if (m_methodId == NULL)
    {
        LOGF("JVM: GetMethodID error.....");
        //Detach主线程
        if(g_jvm->DetachCurrentThread() != JNI_OK)
        {
            LOGE("JVM: DetachCurrentThread failed");
        }
    }

    m_env->CallVoidMethod(g_obj, m_methodId);
    m_env->DeleteLocalRef(m_class);
}

void updateVideoPts(float pts, unsigned long userData)
{
//    LOGD("PTS = %f", pts);

    JNIEnv *m_env;
    jmethodID m_methodId;
    jclass m_class;

    //Attach主线程
    if (g_jvm->AttachCurrentThread(&m_env, NULL) != JNI_OK) {
        LOGE("updateVideoPts  JVM: AttachCurrentThread failed");
        return ;
    }

    //找到对应的类
    m_class = m_env->GetObjectClass(g_obj);
    if (m_class == NULL) {
        LOGD("updateVideoPts JVM: FindClass error.....");
        if(g_jvm->DetachCurrentThread() != JNI_OK)
        {
            LOGE("updateVideoPts JVM: DetachCurrentThread failed");
        }
        return ;
    }

    //再获得类中的方法
    m_methodId = m_env->GetMethodID(m_class, "OnPtsCallback", "(F)V");
    if (m_methodId == NULL)
    {
        LOGF("JVM: GetMethodID error.....");
        //Detach主线程
        if(g_jvm->DetachCurrentThread() != JNI_OK)
        {
            LOGE("JVM: DetachCurrentThread failed");
        }
    }

    m_env->CallVoidMethod(g_obj, m_methodId, pts);
    m_env->DeleteLocalRef(m_class);
}

#ifdef __cplusplus
}
#endif