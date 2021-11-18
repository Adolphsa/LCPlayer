//
// Created by Administrator on 2021/11/17.
//

#include "LCOpenGLShader.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

LCOpenGLShader::LCOpenGLShader()
{
    m_shaderProgram = 0;
}

LCOpenGLShader::~LCOpenGLShader()
{
    
}

void LCOpenGLShader::InitShaderFromFile(AAssetManager*  pManager, const char* vShaderPath, const char* fShaderPath)
{
    GLuint  vertexId = 0;
    GLuint  fragId = 0;
    
    vertexId = compileShader(pManager, vShaderPath, GL_VERTEX_SHADER);
    fragId = compileShader(pManager, fShaderPath, GL_FRAGMENT_SHADER);

    char message[512];
    int status = 0;
    
    m_shaderProgram = glCreateProgram();
    if (vertexId != -1) {
        glAttachShader(m_shaderProgram, vertexId);
    }
    if (fragId != -1) {
        glAttachShader(m_shaderProgram, fragId);
    }
    
    glLinkProgram(m_shaderProgram);

    glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &status);
    if (!status)
    {
        glGetProgramInfoLog(m_shaderProgram, 512, nullptr, message);
        LOGF("Get shaderProgram failed: %s",message);
    }

    LOGD("ShaderProgram success !!!\n");
    
    glDeleteShader(vertexId);
    glDeleteShader(fragId);
}

int LCOpenGLShader::compileShader(AAssetManager*  pAssetManager, const char* sPath, GLint sType)
{
    AAsset* file = AAssetManager_open(pAssetManager, sPath, AASSET_MODE_BUFFER);
    size_t shaderSize = AAsset_getLength(file);
    
    //将shader文件读到buff中
    char* sContentBuff = (char*)malloc(shaderSize);
    AAsset_read(file, sContentBuff, shaderSize);
    LOGD("SHADERS: %s",sContentBuff);
    
    unsigned int shaderID = 0;
    char message[512] = {0};
    int status = 0;
    
    shaderID = glCreateShader(sType);
    glShaderSource(shaderID, 1, &sContentBuff, (const GLint *)&shaderSize);
    glCompileShader(shaderID);
    
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);
    if (!status) {
        //打印shader compile info
        glGetShaderInfoLog(shaderID, 512, NULL, message);
        LOGF("Compile Shader Status failed: %s",message);
    }

    if (sContentBuff != nullptr) {
        free(sContentBuff);
        sContentBuff = nullptr;
    }
    
    AAsset_close(file);
    
    return shaderID;
}

void LCOpenGLShader::Bind()
{
    glUseProgram(m_shaderProgram);
}

void LCOpenGLShader::Release()
{
    glUseProgram(0);
}

void LCOpenGLShader::EnableAttributeArray(const char *name)
{
    GLuint location = glGetAttribLocation(m_shaderProgram, name);
    glEnableVertexAttribArray(location);
}

void LCOpenGLShader::DisableAttributeArray(const char *name)
{
    GLuint location = glGetAttribLocation(m_shaderProgram, name);
    glDisableVertexAttribArray(location);
}

void LCOpenGLShader::SetAttributeBuffer(const char* name, GLenum type, const void *values, int tupleSize, int stride)
{
    GLuint location = glGetAttribLocation(m_shaderProgram, name);
    glVertexAttribPointer(location, tupleSize, type, GL_FALSE, stride, values);
}

void LCOpenGLShader::EnableAttributeArray(int location)
{
    glEnableVertexAttribArray(location);
}

void LCOpenGLShader::DisableAttributeArray(int location)
{
    glDisableVertexAttribArray(location);
}

void LCOpenGLShader::SetAttributeBuffer(int location, GLenum type, const void *values, int tupleSize, int stride)
{
    glVertexAttribPointer(location, tupleSize, type, GL_FALSE, stride, values);
}

void LCOpenGLShader::SetUniformValue(const char* name, int iValue)
{
    glUniform1i(glGetUniformLocation(m_shaderProgram, name), iValue);
}

void LCOpenGLShader::SetUniformValue(const char* name, GLfloat fValue)
{
    glUniform1f(glGetUniformLocation(m_shaderProgram, name), fValue);
}

void LCOpenGLShader::SetUniformValue(const char* name, glm::vec2 vec2Value)
{
    glUniform2fv(glGetUniformLocation(m_shaderProgram, name), 1, (const GLfloat *)glm::value_ptr(vec2Value));
}

void LCOpenGLShader::SetUniformValue(const char* name, glm::vec3 vec3Value)
{
    glUniform3fv(glGetUniformLocation(m_shaderProgram, name), 1, (const GLfloat *)glm::value_ptr(vec3Value));
}

void LCOpenGLShader::SetUniformValue(const char* name, glm::mat4 matValue)
{
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, name) , 1 , GL_FALSE , glm::value_ptr(matValue));
}


