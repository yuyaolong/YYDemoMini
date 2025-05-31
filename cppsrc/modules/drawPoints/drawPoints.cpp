//
// Copyright (c) 2022-present, YYAL.  All rights reserved.
// Created by Yaolong Yu on 2022/8/11.
//

#include "drawPoints.h"
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES3/gl32.h>
#include <cstdlib>
#include <ctime>
#include <cassert>
#include "miscUtils.h"

static int g_winWidth = 0;
static int g_winHeight = 0;
typedef struct _GLData {
    GLuint fbo = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint program = 0;
    EGLDisplay currentDisplay = 0;
    EGLContext currentContext = 0;
    GLuint texID = 0;
    int drawVertexNum = 0;
} GLData;

static GLData g_glData;
static const char g_vtxShaderSrc[] = "#version 320 es\n"
                                     "layout(location = 0) in vec2 a_position;\n"
                                     "void main()\n"
                                     "{\n"
                                     "   gl_PointSize = 20.0;\n"
                                     "   gl_Position = vec4(a_position, 0.0, 1.0);\n"
                                     "}\n";

static const char g_frgShaderSrc[] = "#version 320 es\n"
                                     "precision mediump float;\n"
                                     "uniform vec3 inputColor;\n"
                                     "out vec4 outColor;\n"
                                     "void main()\n"
                                     "{\n"
                                     "   outColor = vec4(inputColor, 1.0f);\n"
                                     "}\n";

static void updateFboRenderTarget()
{
    glBindFramebuffer(GL_FRAMEBUFFER, g_glData.fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_glData.texID, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        ALOGE("FBO not complete");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CHECK_GL_ERROR
}

static void initGLResource()
{
    g_glData.currentDisplay = eglGetCurrentDisplay();
    g_glData.currentContext = eglGetCurrentContext();
    if(g_glData.currentDisplay == EGL_NO_DISPLAY ||
       g_glData.currentContext == EGL_NO_CONTEXT)
    {
        ALOGE("No current EGLDisplay or EGLContext in AADT module");
        return;
    }

    // generate fbo
    glGenFramebuffers(1, &g_glData.fbo);

    // generate vao and vbo
    glGenVertexArrays(1, &g_glData.vao);
    glGenBuffers(1, &g_glData.vbo);
    glBindVertexArray(g_glData.vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_glData.vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    CHECK_GL_ERROR

    // create program
    g_glData.program = glUtils::createProgram(g_vtxShaderSrc, g_frgShaderSrc);
}

static void renderOnFrame(float* pointsData,
                          int dataSize,
                          float colorR,
                          float colorG,
                          float colorB,
                          int lowLeftX,
                          int lowLeftY,
                          int screenWidth,
                          int screenHeight,
                          bool onScreen)
{
    CHECK_GL_ERROR
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glBindBuffer(GL_ARRAY_BUFFER, g_glData.vbo);
    glBufferData(GL_ARRAY_BUFFER, dataSize * sizeof(float), pointsData, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    if (onScreen) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, g_glData.fbo);
    }
    glViewport(lowLeftX, lowLeftY, screenWidth, screenHeight);
//    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
//    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(g_glData.program);CHECK_GL_ERROR
    glUniform3f(glGetUniformLocation(g_glData.program, "inputColor"), colorR, colorG, colorB);CHECK_GL_ERROR
    glBindVertexArray(g_glData.vao);CHECK_GL_ERROR
    glDrawArrays(GL_POINTS, 0, g_glData.drawVertexNum);CHECK_GL_ERROR
    glBindVertexArray(0);CHECK_GL_ERROR
    glBindBuffer(GL_ARRAY_BUFFER, 0);CHECK_GL_ERROR
    glUseProgram(0);CHECK_GL_ERROR
    glBindFramebuffer(GL_FRAMEBUFFER, 0);CHECK_GL_ERROR
    CHECK_GL_ERROR
}

void grReleaseDrawPoints()
{
    glDeleteBuffers(1, &g_glData.vbo);
    glDeleteVertexArrays(1, &g_glData.vao);
    glDeleteFramebuffers(1, &g_glData.fbo);
    glDeleteProgram(g_glData.program);
}

void grInitDrawPoints(int targetWidth,
                      int targetHeight)
{
    assert ((targetWidth  > 0) && (targetHeight > 0));
    g_winWidth = targetWidth;
    g_winHeight = targetHeight;
    initGLResource();
}

void grProcessDrawPoints(unsigned int inPlaceTexID,
                         float* pointsData,
                         int dataSize,
                         float colorR,
                         float colorG,
                         float colorB)
{
    if ((pointsData == NULL) ||
        (dataSize <= 0))
    {
        ALOGE("point data pointer or size wrong");
        return;
    }
    g_glData.texID = inPlaceTexID;
    updateFboRenderTarget();
    g_glData.drawVertexNum = dataSize / 2;
    renderOnFrame(pointsData, dataSize, colorR, colorG, colorB, 0, 0, g_winWidth, g_winHeight, false);
}

void grProcessDrawPointsOnScreen(float* pointsData,
                                 int dataSize,
                                 float colorR,
                                 float colorG,
                                 float colorB,
                                 int lowLeftX,
                                 int lowLeftY,
                                 int screenWidth,
                                 int screenHeight)
{
    if ((pointsData == NULL) ||
        (dataSize <= 0))
    {
        ALOGE("point data pointer or size wrong");
        return;
    }
    g_glData.drawVertexNum = dataSize / 2;
    renderOnFrame(pointsData, dataSize, colorR, colorG, colorB,
                  lowLeftX, lowLeftY, screenWidth, screenHeight, true);
}

