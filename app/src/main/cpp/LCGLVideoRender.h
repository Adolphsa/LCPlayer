//
// Created by Administrator on 2021/11/17.
//

#ifndef LCPLAYER_LCGLVIDEORENDER_H
#define LCPLAYER_LCGLVIDEORENDER_H

#include "LCOpenGLShader.h"
#include "LCNDKCommonDef.h"
#include "LCYUVDataDefine.h"

class LCGLVideoRender {

public:
    LCGLVideoRender();
    ~LCGLVideoRender();

    void InitGL();
    void PaintGL();
    void ResizeGL(int w, int h);

    void SetupAssetManager(AAssetManager *pManager);

    void RendVideo(YUVData_Frame* yuvFrame);

private:
    void loadShaderResources(AAssetManager *pManager);

private:
    AAssetManager* m_pAssetManager;
    LCOpenGLShader* m_pOpenGLShader;

    bool m_bUpdateData = false;

    GLuint m_textures[3];

    int m_nVideoW = 0;
    int m_nVideoH = 0;
    int m_yFrameLength = 0;
    int m_uFrameLength = 0;
    int m_vFrameLength = 0;

    unsigned char* m_pBufYuv420p = nullptr;

    struct LCVertex{
        float x,y,z;
        float u,v;
    };
};


#endif //LCPLAYER_LCGLVIDEORENDER_H
