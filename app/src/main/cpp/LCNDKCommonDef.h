//
// Created by Administrator on 2021/11/9.
//

#ifndef LCPLAYER_LCNDKCOMMONDEF_H
#define LCPLAYER_LCNDKCOMMONDEF_H

#include <string>

#include <GLES3/gl3.h>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include <assert.h>
#include <android/asset_manager_jni.h>
#include <android/asset_manager.h>

#include "LCNDKLogDef.h"

template<typename T>
void LCGLSafePtrDelete(T* ptr)
{
    delete ptr;
    ptr = NULL;
}

class LCGlobalDefine
{
    inline static void PrintGLError()
    {
        GLenum err = glGetError();
        if (err != GL_NO_ERROR)
        {
            LOGD("glError: %s",err);
        }
    }

};

#ifdef __cplusplus
extern "C" {
#endif


struct  LCFloat7{
    float x;
    float y;
    float z;
    float r;
    float g;
    float b;
    float a;
};

struct  LCFloat3{
    float x;
    float y;
    float z;
};

struct  LCFloat4{
    float r;
    float g;
    float b;
    float a;
};

struct  LCFloat5{
    float x;
    float y;
    float z;
    float u;
    float v;
};



#ifdef __cplusplus
}
#endif

#endif //LCPLAYER_LCNDKCOMMONDEF_H
