//
// Created by Administrator on 2021/11/10.
//

#include "LCAudioPlayer.h"
#include "LCNDKLogDef.h"

LCAudioPlayer* LCAudioPlayer::m_pInstance = NULL;
pthread_mutex_t LCAudioPlayer::m_mutex;
LCAudioPlayer::Garbage LCAudioPlayer::m_garbage;

LCAudioPlayer* LCAudioPlayer::GetInstance()
{
    if (m_pInstance == nullptr) {
        pthread_mutex_init(&m_mutex, NULL);

        pthread_mutex_lock(&m_mutex);
        m_pInstance = new LCAudioPlayer();
        pthread_mutex_unlock(&m_mutex);
    }

    return m_pInstance;
}

LCAudioPlayer::LCAudioPlayer()
{
    m_stream = NULL;
}

void LCAudioPlayer::StartAudioPlayer()
{
    pthread_mutex_lock(&m_mutex);

    StopAudioPlayer();
    m_stream = android_OpenAudioDevice(m_sampleRate, m_channel, m_channel, OUT_BUFFER_SIZE);

    pthread_mutex_unlock(&m_mutex);
}

void LCAudioPlayer::StopAudioPlayer()
{
    if(m_stream != NULL){
        pthread_mutex_lock(&m_mutex);
        android_CloseAudioDevice(m_stream);
        m_stream = NULL;
        pthread_mutex_unlock(&m_mutex);
    }
}

int LCAudioPlayer::GetFreeSpace()
{
    pthread_mutex_lock(&m_mutex);
    return m_stream->outBufSamples;
    pthread_mutex_unlock(&m_mutex);
}

bool LCAudioPlayer::WriteAudioData(const char* dataBuff,int size)
{
    if((dataBuff == NULL) || (size <= 0)){
        return false;
    }

    pthread_mutex_lock(&m_mutex);
    int writeSamples = android_AudioOut(m_stream,(short*)dataBuff,size/2);
    pthread_mutex_unlock(&m_mutex);
    //LOGD("WRITE SAMPLES:%d %d",size,writeSamples);
    return true;
}
