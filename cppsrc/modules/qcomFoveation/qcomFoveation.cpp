//
// Copyright (c) 2022-present, YYAL.  All rights reserved.
// Created by Yaolong Yu on 2022/8/11.
//

#include "qcomFoveation.h"
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES3/gl32.h>
#include <cstdlib>
#include <ctime>
#include <cassert>
#include "miscUtils.h"

static int g_winWidth = 0;
static int g_winHeight = 0;
static GLuint g_srcTexID = 0;
typedef struct _GLData {
    GLuint fbo;
    GLuint vao;
    GLuint vbo;
    GLuint program;
    EGLDisplay currentDisplay;
    EGLContext currentContext;
    GLuint targetColorTexID;
    GLuint targetDepthTexID;
    bool useSubSampledLayout;
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
                                     "#extension GL_OVR_multiview2 : require\n"
                                     "layout(num_views = 2) in;\n"
                                     "layout(location = 0) in vec3 a_position;\n"
                                     "layout(location = 1) in vec2 uv_position;\n"
                                     "out vec2 v_texCoor;\n"
                                     "void main()\n"
                                     "{\n"
                                     "   gl_Position = vec4(a_position, 1.0);\n"
                                     "   if (gl_ViewID_OVR == 0u) { \n"
                                     "      v_texCoor = vec2(uv_position.x, uv_position.y);\n"
                                     "   } else {\n"
                                     "      v_texCoor = vec2(uv_position.x, uv_position.y);\n"
                                     "   }\n"
                                     "}\n";

static const char g_frgShaderSrc[] = "#version 320 es\n"
                                     "#extension GL_EXT_fragment_invocation_density : require\n"
                                     "precision mediump float;\n"
                                     "uniform sampler2D srcTex;\n"
                                     "in vec2 v_texCoor;\n"
                                     "out vec4 outColor;\n"
                                     "void main()\n"
                                     "{\n"
                                     "  int levels = gl_FragSizeEXT.x * gl_FragSizeEXT.y;\n"
                                     "  if (levels == 1) { outColor = vec4(1.0, 1.0, 1.0, 1.0); }\n"
                                     "  else if (levels == 2) { outColor = vec4(1.0, 0.0, 0.0, 1.0); }\n"
                                     "  else if (levels == 4) { outColor = vec4(0.0, 1.0, 0.0, 1.0); }\n"
                                     "  else if (levels == 8) { outColor = vec4(0.0, 0.0, 1.0, 1.0); }\n"
                                     "  else if (levels == 16) { outColor = vec4(1.0, 0.0, 1.0, 1.0); }\n"
                                     "  else { outColor = vec4(0.0, 0.0, 0.0, 1.0); }\n"
                                     "  vec4 texColor = texture(srcTex, v_texCoor);\n"
                                 "      outColor = outColor * 0.5 + texColor * 0.5;\n"
                                     "}\n";

static void genTargetTexture(bool useMSAA)
{
    int samples = 0;
    glGetIntegerv(GL_MAX_SAMPLES, &samples);

    g_glData.targetColorTexID = glUtils::createTexture(g_winWidth, g_winHeight,
                                                       GL_TEXTURE_2D_ARRAY, GL_RGBA8, GL_RGBA,
                                                       2, 1,
                                                       GL_UNSIGNED_BYTE, nullptr,
                                                        GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE,
                                                  true, false);
    CHECK_GL_ERROR

    g_glData.targetDepthTexID = glUtils::createTexture(g_winWidth, g_winHeight,
                                                GL_TEXTURE_2D_ARRAY, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT,
                                                       2, 1,
                                                       GL_FLOAT, nullptr,
                                                GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE,
                                                 true, false);

    //    glBindTexture(GL_TEXTURE_2D, g_glData.targetColorTexID);
//    GLint queryFov = 0;
//    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_FOVEATED_FEATURE_QUERY_QCOM, &queryFov);
//    GLint setFov = 0;
//    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_FOVEATED_FEATURE_BITS_QCOM, &setFov);
//    GLfloat fovMiniDensity = 1.0f;
//    glGetTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_FOVEATED_MIN_PIXEL_DENSITY_QCOM, &fovMiniDensity);
//    GLint focalPoint = 0;
//    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_FOVEATED_NUM_FOCAL_POINTS_QUERY_QCOM, &focalPoint);
//    glBindTexture(GL_TEXTURE_2D, 0);
    glUtils::enableColorTexFoveation(GL_TEXTURE_2D_ARRAY, g_glData.targetColorTexID, g_glData.useSubSampledLayout, 0.0f, 0.0f);

    p_glFramebufferTextureMultiviewOVR = (PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVR)eglGetProcAddress ("glFramebufferTextureMultiviewOVR");
    p_glFramebufferTextureMultisampleMultiviewOVR = (PFNGLFRAMEBUFFERTEXTUREMULTISAMPLEMULTIVIEWOVR)eglGetProcAddress ("glFramebufferTextureMultisampleMultiviewOVR");
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, g_glData.fbo);
    if (useMSAA) {
        p_glFramebufferTextureMultisampleMultiviewOVR(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, g_glData.targetColorTexID, 0, 4, 0, 2);CHECK_GL_ERROR
        p_glFramebufferTextureMultisampleMultiviewOVR(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, g_glData.targetDepthTexID, 0, 4, 0, 2);CHECK_GL_ERROR
    } else {
        p_glFramebufferTextureMultiviewOVR(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, g_glData.targetColorTexID, 0, 0, 2);
        p_glFramebufferTextureMultiviewOVR(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, g_glData.targetDepthTexID, 0, 0, 2);
    }
    GLenum res = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    if (res != GL_FRAMEBUFFER_COMPLETE) {
        ALOGE("error code: %d", res);
        ALOGE("FBO not complete");
        abort();
    }
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
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
    genTargetTexture(true);
    CHECK_GL_ERROR

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
    glBindVertexArray(0);
    CHECK_GL_ERROR

    // create program
    g_glData.program = glUtils::createProgram(g_vtxShaderSrc, g_frgShaderSrc);
}

static void renderOnFrame(int showTexture)
{
    CHECK_GL_ERROR
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);

    glBindFramebuffer(GL_FRAMEBUFFER, g_glData.fbo);
    glViewport(0, 0, g_winWidth, g_winHeight);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClearDepthf(1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glUseProgram(g_glData.program);
    glBindVertexArray(g_glData.vao);
    glUniform1i(glGetUniformLocation(g_glData.program, "srcTex"), 0);
    glActiveTexture(GL_TEXTURE0);CHECK_GL_ERROR
    glBindTexture(GL_TEXTURE_2D, g_srcTexID);CHECK_GL_ERROR

    glDrawArrays(GL_TRIANGLES, 0, 6);CHECK_GL_ERROR

    glBindTexture(GL_TEXTURE_2D, 0);CHECK_GL_ERROR
    glBindVertexArray(0);CHECK_GL_ERROR
    glUseProgram(0);CHECK_GL_ERROR
    glBindFramebuffer(GL_FRAMEBUFFER, 0);CHECK_GL_ERROR
}

void grReleaseFoveationQcom()
{
    glDeleteBuffers(1, &g_glData.vbo);
    glDeleteVertexArrays(1, &g_glData.vao);
    glDeleteTextures(1, &g_glData.targetColorTexID);
    glDeleteTextures(1, &g_glData.targetDepthTexID);
    glDeleteFramebuffers(1, &g_glData.fbo);
    glDeleteProgram(g_glData.program);
}

unsigned int grInitFoveationQcom(unsigned int srcTexID,
                                 int targetWidth,
                                 int targetHeight,
                                 bool useSubSampleLayout)
{
    assert(srcTexID != 0);
    assert ((targetWidth >= 0) && (targetHeight >= 0));

    g_glData.useSubSampledLayout = useSubSampleLayout;
    g_winWidth = targetWidth;
    g_winHeight = targetHeight;

    initGLResource();
    CHECK_GL_ERROR

    // update src texture ID
    g_srcTexID = srcTexID;

    return g_glData.targetColorTexID;
}

unsigned int grProcessFoveationQcom(void* pPara,
                                    int focalNum,
                                    int showTexture)
{
    // change foveation parameters dynamically
    glBindTexture(GL_TEXTURE_2D_ARRAY, g_glData.targetColorTexID);
    p_glTextureFoveationParametersQCOM = (PFNGLTextureFoveationParametersQCOM)(eglGetProcAddress("glTextureFoveationParametersQCOM"));
    FoveationParameters* pFovPara = static_cast<FoveationParameters*>(pPara);
    for (int i = 0; i < focalNum; ++i) {
        p_glTextureFoveationParametersQCOM(g_glData.targetColorTexID, 0, i, pFovPara[i].focalX,
                                           pFovPara[i].focalY,
                                          pFovPara[i].gainX,
                                           pFovPara[i].gainY,
                                           pFovPara[i].fovArea);
        p_glTextureFoveationParametersQCOM(g_glData.targetColorTexID, 1, i, pFovPara[i].focalX,
                                           pFovPara[i].focalY,
                                           pFovPara[i].gainX,
                                           pFovPara[i].gainY,
                                           pFovPara[i].fovArea);
        CHECK_GL_ERROR
    }
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    renderOnFrame(showTexture);
    return g_glData.targetDepthTexID;
}

