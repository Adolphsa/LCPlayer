//
// Created by Administrator on 2021/11/9.
//

#include "LCAVCodecHandler.h"
#include "LCNDKLogDef.h"

#if !defined(MIN)
#define MIN(A,B)	((A) < (B) ? (A) : (B))
#endif

LCAVCodecHandler::LCAVCodecHandler()
{
    av_register_all();
    LOGD("av_register_all");

    resetAllMediaPlayerParameters();
}

LCAVCodecHandler::~LCAVCodecHandler()
{

}

void LCAVCodecHandler::SetVideoFilePath(string& path)
{
    m_videoPathString = path;

    std::string fileSuffix = getFileSuffix(m_videoPathString.c_str());
    LOGD("file suffix %s", fileSuffix.c_str());

    if (fileSuffix == "mp3") {
        m_mediaType = MEDIA_TYPE_MUSIC;
    } else {
        m_mediaType = MEDIA_TYPE_VIDEO;
    }
}

std::string  LCAVCodecHandler::GetVideoFilePath()
{
    return m_videoPathString;
}

int LCAVCodecHandler::GetVideoWidth()
{
    return m_videoWidth;
}

int LCAVCodecHandler::GetVideoHeight()
{
    return m_videoHeight;
}

int LCAVCodecHandler::InitVideoCodec()
{
    if (m_videoPathString.empty()) {
        LOGE("file path is Empty...");
        return -1;
    }

    const char* filePath = m_videoPathString.c_str();
    LOGD("file path = %s", filePath);

    int ret = 0;
    if ((ret = avformat_open_input(&m_pFormatCtx, filePath, NULL, NULL)) != 0) {
        LOGE("avformat_open_input fail . %s", av_err2str(ret));
        return -1;
    }

    if (avformat_find_stream_info(m_pFormatCtx, NULL) < 0) {
        LOGE("avformat_find_stream_info failed .");
        return -1;
    }

    av_dump_format(m_pFormatCtx, 0, filePath, 0);

    return 0;
}

void LCAVCodecHandler::UnInitVideoCodec()
{

}

void LCAVCodecHandler::StartPlayVideo()
{

}

void LCAVCodecHandler::StopPlayVideo()
{

}

void LCAVCodecHandler::resetAllMediaPlayerParameters()
{

    m_pFormatCtx       = NULL;
    m_pVideoCodecCtx   = NULL;
    m_pAudioCodecCtx   = NULL;
    m_pYUVFrame        = NULL;
    m_pVideoFrame      = NULL;
    m_pAudioFrame      = NULL;
    m_pAudioSwrCtx     = NULL;
    m_pVideoSwsCtx     = NULL;
    m_pYUV420Buffer    = NULL;
    m_pSwrBuffer       = NULL;

    m_videoWidth   = 0;
    m_videoHeight  = 0;

    m_videoPathString = "";

    m_videoStreamIdx = -1;
    m_audioStreamIdx = -1;

    m_bReadFileEOF   = false;

    m_nSeekingPos      = 0;


    m_nCurrAudioTimeStamp = 0.0f;
    m_nLastAudioTimeStamp = 0.0f;

    m_sampleRate = 48000;
    m_sampleSize = 16;
    m_channel    = 2;

    m_volumeRatio = 1.0f;
    m_swrBuffSize = 0;

    m_vStreamTimeRational = av_make_q(0,0);
    m_aStreamTimeRational = av_make_q(0,0);

    m_mediaType = MEDIA_TYPE_VIDEO;
    m_eMediaPlayStatus = MEDIA_PLAY_STATUS_STOP;
}

std::string LCAVCodecHandler::getFileSuffix(const char* path)
{
    //在参数 str 所指向的字符串中搜索最后一次出现字符 c（一个无符号字符）的位置
    const char* pos = strrchr(path,'.');
    if (pos) {
        std::string str(pos+1);
        std::transform(str.begin(),str.end(),str.begin(),::tolower);
        return str;
    }
    return std::string();
}

