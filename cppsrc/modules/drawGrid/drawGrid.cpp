//
// Copyright (c) 2022-present, YYAL.  All rights reserved.
// Created by Yaolong Yu on 2022/8/11.
//

#include "drawGrid.h"
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES3/gl32.h>
#include <cstdlib>
#include <ctime>
#include <cassert>
#include "miscUtils.h"

typedef struct _GLData {
    GLuint fbo = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint program = 0;
    EGLDisplay currentDisplay = EGL_NO_DISPLAY;
    EGLContext currentContext = EGL_NO_CONTEXT;
    GLuint targetTexID = 0;
    int targetWidth = 0;
    int targetHeight = 0;
    int gridHorizonLines = 0;
    int gridVerticaLines = 0;
    int drawVertices = 0;
    float lineWidth = 10.0f;
} GLData;

static GLData g_glData;
static const char g_vtxShaderSrc[] = "#version 320 es\n"
                                     "layout(location = 0) in vec2 a_position;\n"
                                     "out vec2 v_texCoor;\n"
                                     "void main()\n"
                                     "{\n"
                                     "   gl_Position = vec4(a_position, 0.0, 1.0);\n"
                                     "}\n";

static const char g_frgShaderSrc[] = "#version 320 es\n"
                                     "precision mediump float;\n"
                                     "out vec4 outColor;\n"
                                     "void main()\n"
                                     "{\n"
                                     "  outColor = vec4(0.0, 0.9, 0.0, 1.0);\n"
                                     "}\n";


static void updateRenderTarget()
{
    glBindFramebuffer(GL_FRAMEBUFFER, g_glData.fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_glData.targetTexID, 0);
    GLenum res = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (res != GL_FRAMEBUFFER_COMPLETE) {
        ALOGD("error code: %d", res);
        ALOGE("FBO not complete");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

    int vPosDataSizeInBytes = g_glData.drawVertices * 2 * sizeof(float);

    GLfloat* pVertexData = (GLfloat *) malloc(vPosDataSizeInBytes);
    for (int i = 0; i < g_glData.gridHorizonLines; ++i) {
        float x = -1.0f + 2.0f * float(i) / (float(g_glData.gridHorizonLines) - 1.0f);
        pVertexData[i*4 + 0] = x;
        pVertexData[i*4 + 1] = 1.0f;
        pVertexData[i*4 + 2] = x;
        pVertexData[i*4 + 3] = -1.0f;
    }

    for (int i = 0; i < g_glData.gridVerticaLines; ++i) {
        int base = g_glData.gridHorizonLines * 4;
        float y = -1.0f + 2.0f * float(i) / (float(g_glData.gridVerticaLines) - 1.0f);
        pVertexData[base + i*4 + 0] = 1.0f;
        pVertexData[base + i*4 + 1] = y;
        pVertexData[base + i*4 + 2] = -1.0f;
        pVertexData[base + i*4 + 3] = y;
    }

    // generate fbo
    glGenFramebuffers(1, &g_glData.fbo);

    // generate vao and vbo
    glGenVertexArrays(1, &g_glData.vao);
    glGenBuffers(1, &g_glData.vbo);
    glBindVertexArray(g_glData.vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_glData.vbo);
    glBufferData(GL_ARRAY_BUFFER, vPosDataSizeInBytes, pVertexData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    CHECK_GL_ERROR

    // create program
    g_glData.program = glUtils::createProgram(g_vtxShaderSrc, g_frgShaderSrc);
}

static void renderOnFrame()
{
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, g_glData.fbo);
    glViewport(0, 0, g_glData.targetWidth, g_glData.targetHeight);
//    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
//    glClearDepthf(1.0f);
//    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(g_glData.program);
    glBindVertexArray(g_glData.vao);
    glUniform1i(glGetUniformLocation(g_glData.program, "srcTex"), 0);

    glLineWidth(g_glData.lineWidth);

    glDrawArrays(GL_LINES, 0, g_glData.drawVertices);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void grReleaseDrawGrid()
{
    glDeleteBuffers(1, &g_glData.vbo);
    glDeleteVertexArrays(1, &g_glData.vao);
    glDeleteTextures(1, &g_glData.targetTexID);
    glDeleteFramebuffers(1, &g_glData.fbo);
    glDeleteProgram(g_glData.program);
}

void grInitDrawGrid(int targetWidth,
                    int targetHeight,
                    int gridHorizonNum,
                    int gridVerticalNum)
{
    assert((targetWidth > 0) && (targetHeight > 0));
    assert((gridHorizonNum > 0) && (gridVerticalNum > 0));
    g_glData.targetWidth = targetWidth;
    g_glData.targetHeight = targetHeight;
    g_glData.gridVerticaLines = gridVerticalNum + 1;
    g_glData.gridHorizonLines = gridHorizonNum + 1;
    g_glData.drawVertices = (g_glData.gridVerticaLines + g_glData.gridHorizonLines) * 2;
    initGLResource();
}

void grProcessDrawGrid(unsigned int targetTexID,
                       float lineWidth)
{
    g_glData.targetTexID = targetTexID;
    g_glData.lineWidth = lineWidth;
    updateRenderTarget();
    renderOnFrame();
}

