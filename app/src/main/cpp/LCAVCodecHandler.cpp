//
// Created by Administrator on 2021/11/9.
//

#include "LCAVCodecHandler.h"
#include "LCNDKLogDef.h"
#include "LCAudioPlayer.h"

#if !defined(MIN)
#define MIN(A, B)    ((A) < (B) ? (A) : (B))
#endif

std::atomic<bool> m_bFileThreadRunning(false);
std::atomic<bool> m_bAudioThreadRunning(false);
std::atomic<bool> m_bVideoThreadRunning(false);
std::atomic<bool> m_bThreadRunning(false);

LCAVCodecHandler::LCAVCodecHandler() {
    av_register_all();
    avcodec_register_all();
    LOGD("av_register_all");

    resetAllMediaPlayerParameters();
}

LCAVCodecHandler::~LCAVCodecHandler() {

}

void LCAVCodecHandler::SetVideoFilePath(string &path) {
    m_videoPathString = path;

    std::string fileSuffix = getFileSuffix(m_videoPathString.c_str());
    LOGD("file suffix %s", fileSuffix.c_str());

    if (fileSuffix == "mp3") {
        m_mediaType = MEDIA_TYPE_MUSIC;
    } else {
        m_mediaType = MEDIA_TYPE_VIDEO;
    }
}

std::string LCAVCodecHandler::GetVideoFilePath() {
    return m_videoPathString;
}

int LCAVCodecHandler::GetVideoWidth() {
    return m_videoWidth;
}

int LCAVCodecHandler::GetVideoHeight() {
    return m_videoHeight;
}

int LCAVCodecHandler::InitVideoCodec() {
    if (m_videoPathString.empty()) {
        LOGE("file path is Empty...");
        return -1;
    }

    const char *filePath = m_videoPathString.c_str();
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

    //android中log无法显示
//    av_dump_format(m_pFormatCtx, 0, filePath, 0);

    m_videoStreamIdx = -1;
    m_audioStreamIdx = -1;

    for (int i = 0; i < (int) m_pFormatCtx->nb_streams; ++i) {

        AVCodecParameters *codecParameters = m_pFormatCtx->streams[i]->codecpar;

        if (codecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            m_videoStreamIdx = i;
            LOGD("Video index: %d", m_videoStreamIdx);

            AVCodec *codec = avcodec_find_decoder(codecParameters->codec_id);
            if (codec == nullptr) {
                LOGE("Video AVCodec is NULL");
                return -1;
            }
            LOGD("video codecpar->codec_id = %d ,name = %s", codecParameters->codec_id,
                 avcodec_get_name(codecParameters->codec_id));

            m_pVideoCodecCtx = avcodec_alloc_context3(codec);
            avcodec_parameters_to_context(m_pVideoCodecCtx, codecParameters);

            if ((ret = avcodec_open2(m_pVideoCodecCtx, codec, NULL)) < 0) {
                LOGE("video Could not open codec. %s", av_err2str(ret));
                return -1;
            }

        } else if (codecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
            m_audioStreamIdx = i;
            LOGD("Audio index: %d", m_audioStreamIdx);

            AVCodec *codec = avcodec_find_decoder(codecParameters->codec_id);
            if (codec == nullptr) {
                LOGE("Audio AVCodec is NULL");
                return -1;
            }

            m_pAudioCodecCtx = avcodec_alloc_context3(codec);
            avcodec_parameters_to_context(m_pAudioCodecCtx, codecParameters);

            if (avcodec_open2(m_pAudioCodecCtx, codec, NULL) < 0) {
                LOGE("audio decoder not found.");
                return -1;
            }

            m_sampleRate = m_pAudioCodecCtx->sample_rate;
            m_channel = m_pAudioCodecCtx->channels;
            switch (m_pAudioCodecCtx->sample_fmt) {
                case AV_SAMPLE_FMT_U8:
                    m_sampleSize = 8;
                case AV_SAMPLE_FMT_S16:
                    m_sampleSize = 16;
                    break;
                case AV_SAMPLE_FMT_S32:
                    m_sampleSize = 32;
                    break;
                default:
                    break;
            }

        }
    }

    if (m_pVideoCodecCtx != nullptr) {
        m_pYUVFrame = av_frame_alloc();
        if (m_pYUVFrame == nullptr) {
            LOGE("YUV frame alloc error.");
            return -1;
        }

        m_pYUV420Buffer = (uint8_t *) av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P,
                                                                         m_pVideoCodecCtx->width,
                                                                         m_pVideoCodecCtx->height,
                                                                         1));
        if (m_pYUV420Buffer == nullptr) {
            LOGE("out buffer alloc error.");
            return -1;
        }

        av_image_fill_arrays(m_pYUVFrame->data, m_pYUVFrame->linesize,
                             m_pYUV420Buffer, AV_PIX_FMT_YUV420P, m_pVideoCodecCtx->width,
                             m_pVideoCodecCtx->height, 1);

        m_videoWidth = m_pVideoCodecCtx->width;
        m_videoHeight = m_pVideoCodecCtx->height;
        LOGD("Init VideoCodec success. Width: %d Height: %d", m_videoWidth, m_videoHeight);
    }

    if (m_videoStreamIdx != -1) {
        AVStream *videoStream = m_pFormatCtx->streams[m_videoStreamIdx];
        m_vStreamTimeRational = videoStream->time_base;
        m_videoFPS = videoStream->avg_frame_rate.num / videoStream->avg_frame_rate.den;
        LOGD("Init VideoCodec success.V Time Base: %d  Den :%d", videoStream->time_base.num,
             videoStream->time_base.den);
        LOGD("Init VideoCodec success.m_videoFPS :%d", m_videoFPS);
    }

    if (m_audioStreamIdx != -1) {
        AVStream *audioStream = m_pFormatCtx->streams[m_audioStreamIdx];
        m_aStreamTimeRational = audioStream->time_base;
        LOGD("Init AudioCodec success.A Time Base: %d Den  %d", audioStream->time_base.num,
             audioStream->time_base.den);
    }

    return 0;
}


void LCAVCodecHandler::StartPlayVideo() {

    startMediaProcessThreads();
}

void LCAVCodecHandler::StopPlayVideo() {
    m_bThreadRunning = false;

    m_bReadFileEOF = false;

    m_nCurrAudioTimeStamp = 0.0f;
    m_nLastAudioTimeStamp = 0.0f;

    m_vStreamTimeRational = av_make_q(0, 0);
    m_aStreamTimeRational = av_make_q(0, 0);

    m_eMediaPlayStatus = MEDIA_PLAY_STATUS_STOP;

    LCAudioPlayer::GetInstance()->StopAudioPlayer();

    while (!m_audioPktQueue.isEmpty())
        freePacket(m_audioPktQueue.dequeue());
    while (!m_videoPktQueue.isEmpty())
        freePacket(m_videoPktQueue.dequeue());

    waitAllThreadsExit();

    resetAllMediaPlayerParameters();

    LOGD("======STOP PLAY VIDEO SUCCESS========");
}

void LCAVCodecHandler::startMediaProcessThreads() {
    m_bThreadRunning = true;

    std::thread readThread(&LCAVCodecHandler::doReadMediaFrameThread, this);
    readThread.detach();

    std::thread audioThread(&LCAVCodecHandler::doAudioDecodePlayThread, this);
    audioThread.detach();

    std::thread videoThread(&LCAVCodecHandler::doVideoDecodeShowThread, this);
    videoThread.detach();
}

void LCAVCodecHandler::SeekMedia(float nPos) {

}

void LCAVCodecHandler::waitAllThreadsExit() {
    while (m_bFileThreadRunning) {
        stdThreadSleep(10);
        continue;
    }

    while (m_bAudioThreadRunning) {
        stdThreadSleep(10);
        continue;
    }

    while (m_bVideoThreadRunning) {
        stdThreadSleep(10);
        continue;
    }
}

float LCAVCodecHandler::GetMediaTotalSeconds() {
    float totalDuration = m_pFormatCtx->duration / (AV_TIME_BASE * 1.000);
    return totalDuration;
}

void LCAVCodecHandler::SetMediaStatusPlay() {
    m_eMediaPlayStatus = MEDIA_PLAY_STATUS_PLAYING;
}

void LCAVCodecHandler::SetMediaStatusPause() {
    m_eMediaPlayStatus = MEDIA_PLAY_STATUS_PAUSE;
}

int LCAVCodecHandler::GetPlayerStatus() {
    return m_eMediaPlayStatus;
}

void LCAVCodecHandler::tickVideoFrameTimerDelay(int64_t pts) {

}

void LCAVCodecHandler::tickAudioFrameTimerDelay(int64_t pts) {

}

void LCAVCodecHandler::doReadMediaFrameThread() {
    while (m_bThreadRunning) {
        m_bFileThreadRunning = true;

        if (m_eMediaPlayStatus == MEDIA_PLAY_STATUS_PAUSE) {
            stdThreadSleep(10);
            continue;
        }

        if (m_pVideoCodecCtx != NULL && m_pAudioCodecCtx != NULL) { //有视频,有音频

            if (m_videoPktQueue.size() > MAX_VIDEO_FRAME_IN_QUEUE &&
                m_audioPktQueue.size() > MAX_AUDIO_FRAME_IN_QUEUE) {
                stdThreadSleep(10);
                continue;
            }

        } else if (m_pVideoCodecCtx != NULL && m_pAudioCodecCtx == NULL) { //只有视频,没有音频

            float sleepTime = 1000.0 / (float) m_videoFPS;
            stdThreadSleep((int) sleepTime);

        } else { //只有音频

            if (m_videoPktQueue.size() > MAX_AUDIO_FRAME_IN_QUEUE) {
                stdThreadSleep(10);
                continue;
            }
        }

        //没有读到文件结尾就一直读
        if (!m_bReadFileEOF) {
            readMediaPacket();
        } else {
            stdThreadSleep(10);
        }

    }

    LOGD("read file thread exit...");

    m_bFileThreadRunning = false;

    return;
}

void LCAVCodecHandler::readMediaPacket() {
    AVPacket *packet = (AVPacket *) malloc(sizeof(AVPacket));
    if (!packet) {
        return;
    }

    av_init_packet(packet);

    m_eMediaPlayStatus = MEDIA_PLAY_STATUS_PLAYING;

    int retValue = av_read_frame(m_pFormatCtx, packet);
    if (retValue == 0) {

        if (packet->stream_index == m_videoStreamIdx) { //video frame

            if (!av_dup_packet(packet)) {

                //packet enqueue
                m_videoPktQueue.enqueue(packet);

            } else {
                freePacket(packet);
            }
        }

        if (packet->stream_index == m_audioStreamIdx) { //Audio frame
            if (!av_dup_packet(packet)) {

                m_audioPktQueue.enqueue(packet);

            } else {
                freePacket(packet);
            }
        }
    } else if (retValue < 0) {
        if ((m_bReadFileEOF == false) && (retValue == AVERROR_EOF)) {
            m_bReadFileEOF = true;
        }
        return;
    }
}

float LCAVCodecHandler::getAudioTimeStampFromPTS(int64 pts) {
    float aTimeStamp = pts * av_q2d(m_aStreamTimeRational);
    return aTimeStamp;
}

float LCAVCodecHandler::getVideoTimeStampFromPTS(int64 pts) {
    float vTimeStamp = pts * av_q2d(m_vStreamTimeRational);
    return vTimeStamp;
}

void LCAVCodecHandler::doVideoDecodeShowThread() {
    if (m_pFormatCtx == NULL) {
        return;
    }

    if (m_pVideoFrame == NULL) {
        m_pVideoFrame = av_frame_alloc();
    }

    while (m_bThreadRunning) {

        m_bVideoThreadRunning = true;

        if (m_eMediaPlayStatus == MEDIA_PLAY_STATUS_PAUSE) {
            stdThreadSleep(10);
            continue;
        }

        if (m_videoPktQueue.isEmpty()) {
            stdThreadSleep(1);
            continue;
        }

        AVPacket *pkt = (AVPacket *) m_videoPktQueue.dequeue();
        if (pkt == NULL) {
            break;
        }

        if (!m_bThreadRunning) {
            freePacket(pkt);
            break;
        }

        tickVideoFrameTimerDelay(pkt->pts);

        int retValue = avcodec_send_packet(m_pVideoCodecCtx, pkt);
        if (retValue != 0) {
            freePacket(pkt);
            continue;
        }

        int decodeRet = avcodec_receive_frame(m_pVideoCodecCtx, m_pVideoFrame);
        if (decodeRet == 0) {
            convertAndRenderVideo(m_pVideoFrame, pkt->pts);
        }

        freePacket(pkt);
    }

    LOGD("video decode show  thread exit...");

    m_bVideoThreadRunning = false;

    return;
}

void LCAVCodecHandler::doAudioDecodePlayThread() {
    if (m_pFormatCtx == NULL) {
        return;
    }

    if (m_pAudioFrame == NULL) {
        m_pAudioFrame = av_frame_alloc();
    }

    while (m_bThreadRunning) {
        m_bAudioThreadRunning = true;

        if (m_eMediaPlayStatus == MEDIA_PLAY_STATUS_PAUSE) {
            stdThreadSleep(10);
            continue;
        }

        if (m_audioPktQueue.isEmpty()) {
            stdThreadSleep(1);
            continue;
        }

        AVPacket *pkt = (AVPacket *) m_audioPktQueue.dequeue();
        if (pkt == NULL) {
            break;
        }

        if (!m_bThreadRunning) {
            freePacket(pkt);
            break;
        }

        tickAudioFrameTimerDelay(pkt->pts);

        int retValue = avcodec_send_packet(m_pAudioCodecCtx, pkt);
        if (retValue != 0) {
            freePacket(pkt);
            continue;
        }

        int decodeRet = avcodec_receive_frame(m_pAudioCodecCtx, m_pAudioFrame);
        if (decodeRet == 0) {
            convertAndPlayAudio(m_pAudioFrame);
        }

        freePacket(pkt);
    }

    LOGD("audio decode show  thread exit...");

    m_bAudioThreadRunning = false;

    return;
}

void LCAVCodecHandler::convertAndRenderVideo(AVFrame *videoFrame, long long ppts) {
    if (videoFrame == NULL) {
        return;
    }

    if (m_pVideoSwsCtx == NULL) {
        m_pVideoSwsCtx = sws_getContext(m_pVideoCodecCtx->width, m_pVideoCodecCtx->height,
                                        m_pVideoCodecCtx->pix_fmt,
                                        m_pVideoCodecCtx->width, m_pVideoCodecCtx->height,
                                        AV_PIX_FMT_YUV420P,
                                        SWS_BICUBIC,
                                        NULL, NULL, NULL);
    }

    sws_scale(m_pVideoSwsCtx,
              (const uint8_t *const *) videoFrame->data,
              videoFrame->linesize,
              0, m_pVideoCodecCtx->height,
              m_pYUVFrame->data,
              m_pYUVFrame->linesize);

    //y lenght
    unsigned int lumaLength = m_pVideoCodecCtx->height * (MIN(videoFrame->linesize[0], m_pVideoCodecCtx->width));
    //u length
    unsigned int chromBLength=((m_pVideoCodecCtx->height)/2)*(MIN(videoFrame->linesize[1], (m_pVideoCodecCtx->width)/2));
    //v length
    unsigned int chromRLength=((m_pVideoCodecCtx->height)/2)*(MIN(videoFrame->linesize[2], (m_pVideoCodecCtx->width)/2));

    YUVData_Frame *updateYUVFrame = new YUVData_Frame();

    updateYUVFrame->luma.length = lumaLength;
    updateYUVFrame->chromaB.length = chromBLength;
    updateYUVFrame->chromaR.length = chromRLength;

    updateYUVFrame->luma.dataBuffer = (unsigned char*)malloc(lumaLength);
    updateYUVFrame->chromaB.dataBuffer=(unsigned char*)malloc(chromBLength);
    updateYUVFrame->chromaR.dataBuffer=(unsigned char*)malloc(chromRLength);

    //需要把数据复制出来的原因是opengl是一个单线程的工作环境，直接把数据传过去会有问题
    copyDecodedFrame420(m_pYUVFrame->data[0], updateYUVFrame->luma.dataBuffer, m_pYUVFrame->linesize[0],
                        m_pVideoCodecCtx->width, m_pVideoCodecCtx->height);
    copyDecodedFrame420(m_pYUVFrame->data[1], updateYUVFrame->chromaB.dataBuffer,m_pYUVFrame->linesize[1],
                        m_pVideoCodecCtx->width / 2,m_pVideoCodecCtx->height / 2);
    copyDecodedFrame420(m_pYUVFrame->data[2], updateYUVFrame->chromaR.dataBuffer,m_pYUVFrame->linesize[2],
                        m_pVideoCodecCtx->width / 2,m_pVideoCodecCtx->height / 2);

    updateYUVFrame->width = m_pVideoCodecCtx->width;
    updateYUVFrame->height = m_pVideoCodecCtx->height;

    updateYUVFrame->pts = ppts;


    if(m_updateVideoCallback)
    {
        m_updateVideoCallback(updateYUVFrame,m_userDataVideo);
    }

    if(updateYUVFrame->luma.dataBuffer){
        free(updateYUVFrame->luma.dataBuffer);
        updateYUVFrame->luma.dataBuffer=NULL;
    }

    if(updateYUVFrame->chromaB.dataBuffer){
        free(updateYUVFrame->chromaB.dataBuffer);
        updateYUVFrame->chromaB.dataBuffer=NULL;
    }

    if(updateYUVFrame->chromaR.dataBuffer){
        free(updateYUVFrame->chromaR.dataBuffer);
        updateYUVFrame->chromaR.dataBuffer=NULL;
    }

    if(updateYUVFrame){
        delete updateYUVFrame;
        updateYUVFrame = NULL;
    }

}

void LCAVCodecHandler::convertAndPlayAudio(AVFrame *decodedFrame) {
    if (!m_pFormatCtx || !m_pAudioFrame || !decodedFrame)
    {
        return ;
    }

    if (m_pAudioSwrCtx == NULL) {
        m_pAudioSwrCtx = swr_alloc();

        swr_alloc_set_opts(m_pAudioSwrCtx,
                           av_get_default_channel_layout(m_channel),
                           AV_SAMPLE_FMT_S16,
                           m_sampleRate,
                           av_get_default_channel_layout(m_pAudioCodecCtx->channels),
                           m_pAudioCodecCtx->sample_fmt,
                           m_pAudioCodecCtx->sample_rate,
                           0, NULL);

        if (m_pAudioSwrCtx != NULL) {
            int ret = swr_init(m_pAudioSwrCtx);
            LOGD("swr_init RetValue: %d", ret);
            //qDebug()<<"swr_init RetValue:"<<ret;
        }
    }

    int bufSize = av_samples_get_buffer_size(NULL, m_channel, decodedFrame->nb_samples, AV_SAMPLE_FMT_S16, 0);

    if (!m_pSwrBuffer || m_swrBuffSize < bufSize) {
        m_swrBuffSize = bufSize;
        m_pSwrBuffer = (uint8_t *)realloc(m_pSwrBuffer, m_swrBuffSize);
    }

    uint8_t *outbuf[2] = { m_pSwrBuffer, 0 };

    int len = swr_convert(m_pAudioSwrCtx, outbuf,
                          decodedFrame->nb_samples,
                          (const uint8_t **)decodedFrame->data,
                          decodedFrame->nb_samples);
    if (len <= 0) {
        return;
    }

    if( !m_bThreadRunning ){
        return;
    }

    LCAudioPlayer::GetInstance()->WriteAudioData((const char*)m_pSwrBuffer,bufSize);

}


void LCAVCodecHandler::copyDecodedFrame420(uint8_t *src, uint8_t *dist, int linesize, int width,
                                           int height) {

    width = MIN(linesize, width);
    for (int i = 0; i < height; ++i) {
        memcpy(dist, src, width);
        dist += width;
        src += linesize;
    }

}

void LCAVCodecHandler::copyDecodedFrame(uint8_t *src, uint8_t *dist, int linesize, int width,
                                        int height) {
    width = MIN(linesize, width);

    for (int i = 0; i < height; ++i) {
        memcpy(dist, src, width);
        dist += width;
        src += linesize;
    }
}

void LCAVCodecHandler::UnInitVideoCodec() {
    LOGD("UnInitVideoCodec...");


}

void LCAVCodecHandler::freePacket(AVPacket *pkt) {
    if (pkt == NULL) {
        return;
    }

    av_free_packet(pkt);
    free(pkt);
}

void LCAVCodecHandler::stdThreadSleep(int mseconds) {
    std::chrono::milliseconds sleepTime(mseconds);
    std::this_thread::sleep_for(sleepTime);
}

void LCAVCodecHandler::resetAllMediaPlayerParameters() {

    m_pFormatCtx = NULL;
    m_pVideoCodecCtx = NULL;
    m_pAudioCodecCtx = NULL;
    m_pYUVFrame = NULL;
    m_pVideoFrame = NULL;
    m_pAudioFrame = NULL;
    m_pAudioSwrCtx = NULL;
    m_pVideoSwsCtx = NULL;
    m_pYUV420Buffer = NULL;
    m_pSwrBuffer = NULL;

    m_videoWidth = 0;
    m_videoHeight = 0;

    m_videoPathString = "";

    m_videoStreamIdx = -1;
    m_audioStreamIdx = -1;

    m_bReadFileEOF = false;

    m_nSeekingPos = 0;


    m_nCurrAudioTimeStamp = 0.0f;
    m_nLastAudioTimeStamp = 0.0f;

    m_sampleRate = 48000;
    m_sampleSize = 16;
    m_channel = 2;

    m_volumeRatio = 1.0f;
    m_swrBuffSize = 0;

    m_vStreamTimeRational = av_make_q(0, 0);
    m_aStreamTimeRational = av_make_q(0, 0);

    m_mediaType = MEDIA_TYPE_VIDEO;
    m_eMediaPlayStatus = MEDIA_PLAY_STATUS_STOP;
}

std::string LCAVCodecHandler::getFileSuffix(const char *path) {
    //在参数 str 所指向的字符串中搜索最后一次出现字符 c（一个无符号字符）的位置
    const char *pos = strrchr(path, '.');
    if (pos) {
        std::string str(pos + 1);
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
    }
    return std::string();
}

