//
// Copyright (c) 2022-present, YYAL.  All rights reserved.
// Created by Yaolong Yu on 2022/8/11.
//

#include "showOnScreenMultiview.h"
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES3/gl32.h>
#include <cstdlib>
#include <ctime>
#include <cassert>
#include "miscUtils.h"

typedef struct _GLData {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint program = 0;
    EGLDisplay currentDisplay = EGL_NO_DISPLAY;
    EGLContext currentContext = EGL_NO_CONTEXT;
    EGLContext currentDrawSurface = EGL_NO_SURFACE;
    GLuint showTex2DArrayID = 0;
    GLuint showTex2DArrayDepthID = 0;
    int screenWidth = 0;
    int screenHeight = 0;
    bool useSubSampleLayout = false;
} GLData;

static GLData g_glData;
static const int LAYOUT_VERTEX_POS_DATA_FLOATS_NUM = 24; // pos is vec2 floats
static GLfloat VERTEX_DATA[LAYOUT_VERTEX_POS_DATA_FLOATS_NUM] = {-1.0f, -1.0f, 0.0f, 0.0f,
                                                                  1.0f, -1.0f, 1.0f, 0.0f,
                                                                  1.0f,  1.0f, 1.0f, 1.0f,
                                                                 -1.0f, -1.0f, 0.0f, 0.0f,
                                                                  1.0f,  1.0f, 1.0f, 1.0f,
                                                                 -1.0f,  1.0f, 0.0f, 1.0f};
static const char g_vtxShaderSrc[] = "#version 320 es\n"
                                     "layout(location = 0) in vec2 a_position;\n"
                                     "layout(location = 1) in vec2 uv_position;\n"
                                     "out vec2 v_texCoor;\n"
                                     "void main()\n"
                                     "{\n"
                                     "   gl_Position = vec4(a_position, 0.0, 1.0);\n"
                                     "   v_texCoor = vec2(uv_position.x, uv_position.y);\n"
                                     "}\n";

static const char g_frgShaderSrc[] = "#version 320 es\n"
                                     "#extension GL_QCOM_texture_foveated_subsampled_layout : require\n"
                                     "precision mediump float;\n"
                                     "mediump uniform sampler2DArray srcTex;\n"
                                     "mediump uniform sampler2DArray srcDepthTex;\n"
                                     "uniform float showDepth;"
                                     "in vec2 v_texCoor;\n"
                                     "out vec4 outColor;\n"
                                     "void main()\n"
                                     "{\n"
                                     "  if (v_texCoor.x < 0.5) {\n"
                                     "      outColor = texture(srcTex, vec3(v_texCoor.x * 2.0, v_texCoor.y, 0.0));\n"
                                     "      if (showDepth > 0.0) {\n"
                                     "          float dValue = texture(srcDepthTex, vec3(v_texCoor.x * 2.0, v_texCoor.y, 0.0)).r;\n"
                                     "          outColor = vec4(vec3(dValue), 1.0);\n"
                                     "      }\n"
                                     "  } else {\n"
                                     "      outColor = texture(srcTex, vec3((v_texCoor.x - 0.5) * 2.0, v_texCoor.y, 1.0));\n"
                                     "      // float dValue = texture(srcDepthTex, vec3((v_texCoor.x - 0.5) * 2.0, v_texCoor.y, 1.0)).r;\n"
                                     "      // outColor = vec4(vec3(dValue), 1.0);\n"
                                     "  }\n"
                                     "}\n";

static const char g_frgShaderSrc_subsampleLayout[] = "#version 320 es\n"
                                     "#extension GL_QCOM_texture_foveated_subsampled_layout : require\n"
                                     "precision mediump float;\n"
                                     "layout(subsampled) mediump uniform sampler2DArray srcTex;\n"
                                     "mediump uniform sampler2DArray srcDepthTex;\n"
                                     "uniform float showDepth;"
                                     "in vec2 v_texCoor;\n"
                                     "out vec4 outColor;\n"
                                     "void main()\n"
                                     "{\n"
                                     "  if (v_texCoor.x < 0.5) {\n"
                                     "      outColor = texture(srcTex, vec3(v_texCoor.x * 2.0, v_texCoor.y, 0.0));\n"
                                     "      if (showDepth > 0.0) {\n"
                                     "          float dValue = texture(srcDepthTex, vec3(v_texCoor.x * 2.0, v_texCoor.y, 0.0)).r;\n"
                                     "          outColor = vec4(vec3(dValue), 1.0);\n"
                                     "      }\n"
                                     "  } else {\n"
                                     "      outColor = texture(srcTex, vec3((v_texCoor.x - 0.5) * 2.0, v_texCoor.y, 1.0));\n"
                                     "      // float dValue = texture(srcDepthTex, vec3((v_texCoor.x - 0.5) * 2.0, v_texCoor.y, 1.0)).r;\n"
                                     "      // outColor = vec4(vec3(dValue), 1.0);\n"
                                     "  }\n"
                                     "}\n";

static void initGLResource()
{
    g_glData.currentDisplay = eglGetCurrentDisplay();
    g_glData.currentContext = eglGetCurrentContext();
    g_glData.currentDrawSurface = eglGetCurrentSurface(EGL_DRAW);
    if(g_glData.currentDisplay == EGL_NO_DISPLAY ||
       g_glData.currentContext == EGL_NO_CONTEXT)
    {
        ALOGE("No current EGLDisplay or EGLContext in AADT module");
        return;
    }

    // generate vao and vbo
    glGenVertexArrays(1, &g_glData.vao);
    glGenBuffers(1, &g_glData.vbo);
    glBindVertexArray(g_glData.vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_glData.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VERTEX_DATA), VERTEX_DATA, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);CHECK_GL_ERROR

    // create program
    g_glData.program = glUtils::createProgram(g_vtxShaderSrc, g_glData.useSubSampleLayout ? g_frgShaderSrc_subsampleLayout : g_frgShaderSrc);
}

static void renderOnFrame(int lowLeftX,
                          int lowLeftY,
                          int targetWidth,
                          int targetHeight,
                          bool showDepth)
{
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER,0);
    glViewport(lowLeftX, lowLeftY, targetWidth, targetHeight);
//    glClearColor(0.5f, 0.5f, 0.8f, 1.0f);
//    glClear(GL_COLOR_BUFFER_BIT);
//    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
//    glClearDepthf(1.0f);

    glUseProgram(g_glData.program);
    glBindVertexArray(g_glData.vao);
    glUniform1i(glGetUniformLocation(g_glData.program, "srcTex"), 0);
    glUniform1i(glGetUniformLocation(g_glData.program, "srcDepthTex"), 1);
    glUniform1f(glGetUniformLocation(g_glData.program, "showDepth"), showDepth ? 1.0f: 0.0f);
    CHECK_GL_ERROR
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, g_glData.showTex2DArrayID);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, g_glData.showTex2DArrayDepthID);

    glDrawArrays(GL_TRIANGLES, 0, 6);


    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void grReleaseShowOnScreenMultiview()
{
    glDeleteBuffers(1, &g_glData.vbo);
    glDeleteVertexArrays(1, &g_glData.vao);
    glDeleteProgram(g_glData.program);
}

void grInitShowOnScreenMultiview(bool useSubSampleLayout)
{
    g_glData.useSubSampleLayout = useSubSampleLayout;
    initGLResource();
}

void grProcessShowOnScreenMultiview(unsigned int showTex2DArrayID,
                                    unsigned int showTex2DArrayDepthID,
                                   int lowLeftX,
                                   int lowLeftY,
                                   int screenWidth,
                                   int screenHeight,
                                   bool showDepth)
{
    assert(showTex2DArrayID != 0);
    assert((lowLeftX >= 0)    &&
           (lowLeftY >= 0)    &&
           (screenWidth > 0) &&
           (screenHeight > 0));
    g_glData.showTex2DArrayID = showTex2DArrayID;
    g_glData.showTex2DArrayDepthID = showTex2DArrayDepthID;

    renderOnFrame(lowLeftX,
                  lowLeftY,
                  screenWidth,
                  screenHeight,
                  showDepth);
}

