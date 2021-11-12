//
// Created by Administrator on 2021/11/10.
//

#ifndef LCPLAYER_LCAUDIOPLAYER_H
#define LCPLAYER_LCAUDIOPLAYER_H

#include <mutex>
#include "OpenSLInterface.h"

#define OUT_BUFFER_SIZE 8192

class LCAudioPlayer {
public:

    static LCAudioPlayer* GetInstance();

    void StartAudioPlayer();
    void StopAudioPlayer();

    int GetFreeSpace();
    bool WriteAudioData(const char* dataBuff,int size);

    void SetSampleRate(int value){m_sampleRate = value;}
    void SetSampleSize(int value){m_sampleSize = value;}
    void Setchannel(int value){m_channel = value;}

private:
    LCAudioPlayer();

    int m_sampleRate = 44100;
    int m_sampleSize = 16;
    int m_channel = 2;

    static LCAudioPlayer*  m_pInstance;
    static pthread_mutex_t m_mutex;

    OPENSL_STREAM* m_stream;

    class Garbage
    {
    public:
        ~Garbage(){
            if(LCAudioPlayer::m_pInstance){
                delete LCAudioPlayer::m_pInstance;
                LCAudioPlayer::m_pInstance = NULL;
            }
        }
    };
    static Garbage  m_garbage;
};


#endif //LCPLAYER_LCAUDIOPLAYER_H
