//
// Created by Administrator on 2021/11/17.
//

#ifndef LCPLAYER_LCOPENGLSHADER_H
#define LCPLAYER_LCOPENGLSHADER_H

#include "LCNDKCommonDef.h"

class LCOpenGLShader {
public:
    LCOpenGLShader();
    ~LCOpenGLShader();

    void Bind();
    void Release();

    void InitShaderFromFile(AAssetManager*  pManager, const char* vShaderPath,const char* fShaderPath);

    void EnableAttributeArray(const char *name);
    void DisableAttributeArray(const char *name);
    void SetAttributeBuffer(const char* name, GLenum type, const void *values, int tupleSize, int stride = 0);

    void EnableAttributeArray(int location);
    void DisableAttributeArray(int location);
    void SetAttributeBuffer(int location, GLenum type, const void *values, int tupleSize, int stride = 0);

    void SetUniformValue(const char* name, int iValue);
    void SetUniformValue(const char* name, GLfloat fValue);
    void SetUniformValue(const char* name, glm::vec2 vec2Value);
    void SetUniformValue(const char* name, glm::vec3 vec3Value);
    void SetUniformValue(const char* name, glm::mat4 matValue);

private:
    int compileShader(AAssetManager*  pAssetManager, const char* sPath, GLint sType);

private:
    GLuint m_shaderProgram;

};


#endif //LCPLAYER_LCOPENGLSHADER_H
