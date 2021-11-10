//
// Created by Administrator on 2021/11/9.
//

#ifndef LCPLAYER_LCYUVDATADEFINE_H
#define LCPLAYER_LCYUVDATADEFINE_H

#include <stdint.h>
#include <stdio.h>

//把原来对齐方式设置压栈，并设新的对齐方式设置为1个字节对齐
#pragma pack(push, 1)

#define MAX_AUDIO_FRAME_IN_QUEUE    1200
#define MAX_VIDEO_FRAME_IN_QUEUE    600

typedef struct YUVChannelDef
{
    unsigned int    length;
    unsigned char*  dataBuffer;

}YUVChannel;

typedef struct  YUVFrameDef
{
    unsigned int    width;
    unsigned int    height;
    YUVChannel      luma;
    YUVChannel      chromaB;
    YUVChannel      chromaR;
    long long       pts;

}YUVData_Frame;

typedef struct DecodedAudiodataDef
{
    unsigned char*  dataBuff;
    unsigned int    dataLength;
}LCDecodedAudioData;

//恢复对齐状态
#pragma pack(pop)

#endif //LCPLAYER_LCYUVDATADEFINE_H
