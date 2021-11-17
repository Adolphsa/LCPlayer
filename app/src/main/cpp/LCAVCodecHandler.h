//
// Created by Administrator on 2021/11/9.
//

#ifndef LCPLAYER_LCAVCODECHANDLER_H
#define LCPLAYER_LCAVCODECHANDLER_H

#include <mutex>
#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include "LCYUVDataDefine.h"
#include "LCQueueDef.h"

using namespace std;


extern "C" {
#include<libavcodec/avcodec.h>
#include<libavformat/avformat.h>
#include<libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/imgutils.h>
}

#ifndef int64
#define int64 long long
#endif

enum LCMediaType{
    MEDIA_TYPE_VIDEO = 0,
    MEDIA_TYPE_MUSIC
};

enum MediaPlayStatus
{
    MEDIA_PLAY_STATUS_PLAYING = 0,
    MEDIA_PLAY_STATUS_PAUSE,
    MEDIA_PLAY_STATUS_SEEK,
    MEDIA_PLAY_STATUS_STOP
};

typedef void  (*UpdateVideo2GUI_Callback)    (YUVData_Frame* yuv,unsigned long userData);
typedef void  (*UpdateCurrentPTS_Callback)   (float pts,unsigned long userData);


class LCAVCodecHandler {
public:
    explicit LCAVCodecHandler();
    virtual ~LCAVCodecHandler();

    void SetVideoFilePath(string& path);
    std::string  GetVideoFilePath();

    int GetVideoWidth();
    int GetVideoHeight();

    int InitVideoCodec();
    void UnInitVideoCodec();

    void StartPlayVideo();
    void StopPlayVideo();

    void SetMediaStatusPlay();
    void SetMediaStatusPause();

    void SeekMedia(float nPos);// mediaType 0 视频 1音乐

    float GetMediaTotalSeconds();

    int GetPlayerStatus();

    void SetupUpdateVideoCallback(UpdateVideo2GUI_Callback callback, unsigned long userData);
    void SetupUpdateCurrentPTSCallback(UpdateCurrentPTS_Callback callback, unsigned long userData);

private:

    void convertAndRenderVideo(AVFrame* decodedFrame,long long ppts);
    void convertAndPlayAudio(AVFrame* decodedFrame);
    void copyDecodedFrame(uint8_t* src, uint8_t* dist,int linesize, int width, int height);
    void copyDecodedFrame420(uint8_t* src, uint8_t* dist,int linesize, int width, int height);

    void decodeAndRenderVideo();

    void tickVideoFrameTimerDelay(int64_t pts);
    void tickAudioFrameTimerDelay(int64_t pts);

    void doReadMediaFrameThread();
    void doAudioDecodePlayThread();
    void doVideoDecodeShowThread();

    void readMediaPacket();
    void freePacket(AVPacket* pkt);

    float getAudioTimeStampFromPTS(int64 pts);
    float getVideoTimeStampFromPTS(int64 pts);

    void startMediaProcessThreads();
    void waitAllThreadsExit();

    void stdThreadSleep(int mseconds);

    void resetAllMediaPlayerParameters();

    std::string getFileSuffix(const char* path);

private:
    int m_videoWidth = 0;
    int m_videoHeight = 0;

    string m_videoPathString = "";

    int m_videoStreamIdx = -1;
    int m_audioStreamIdx = -1;

    bool m_bReadFileEOF = false;

    int64 m_nSeekingPos = 0;

    bool m_bVideoSeekingOk = false;


    unsigned long               m_userDataVideo = 0;
    UpdateVideo2GUI_Callback    m_updateVideoCallback = NULL;

    unsigned long               m_userDataPts = 0;
    UpdateCurrentPTS_Callback   m_updateCurrentPTSCallback = NULL;

    MediaPlayStatus m_eMediaPlayStatus;

    AVFormatContext* m_pFormatCtx = NULL;

    AVCodecContext* m_pVideoCodecCtx = NULL;
    AVCodecContext* m_pAudioCodecCtx = NULL;

    AVFrame*  m_pYUVFrame = NULL;

    AVFrame* m_pVideoFrame = NULL;
    AVFrame* m_pAudioFrame = NULL;

    SwrContext* m_pAudioSwrCtx = NULL;
    SwsContext* m_pVideoSwsCtx = NULL;

    int m_videoFPS = 0;
    uint8_t*  m_pYUV420Buffer = NULL;

    float m_nCurrAudioTimeStamp = 0.0f;
    float  m_nLastAudioTimeStamp = 0.0f;

    AVRational m_vStreamTimeRational;
    AVRational m_aStreamTimeRational;

    int m_sampleRate = 48000;
    int m_sampleSize = 16;
    int m_channel = 2;
    float m_volumeRatio = 1.00;

    uint8_t* m_pSwrBuffer = NULL;
    int m_swrBuffSize = 0;

    LCMediaQueue<AVPacket *> m_audioPktQueue;
    LCMediaQueue<AVPacket *> m_videoPktQueue;

    LCMediaType         m_mediaType = MEDIA_TYPE_VIDEO;
};


#endif //LCPLAYER_LCAVCODECHANDLER_H
