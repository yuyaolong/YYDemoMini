//
// Copyright (c) 2022-present, YYAL.  All rights reserved.
// Created by Yaolong Yu on 2022/8/11.
//

#include "foveationByVRS.h"
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
    GLuint posVbo;
    GLuint uvVbo;
    GLuint ebo;
    GLuint program;
    EGLDisplay currentDisplay;
    EGLContext currentContext;
    GLuint targetTexID;
    GLuint depthRbo;
} GLData;

static GLData g_glData;
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
                                     "#extension GL_EXT_fragment_invocation_density : require\n"
                                     "precision mediump float;\n"
                                     "uniform int showColor;"
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
                                     "  if (showColor == 1) { \n"
                                     "      outColor = outColor * 0.5 + texColor * 0.5;\n"
                                     "  } else { \n"
                                     "      outColor = texColor;} \n"
                                     "   //outColor = vec4( float( * 10) / 255.0, float(gl_FragSizeEXT.y * 10) / 255.0, 0.0, 1.0);\n"
                                     "}\n";

static const int GridHorNum = 60;
static const int GridVerNum = 60;
static const float GridWidth = g_winWidth * 1.0f / GridHorNum;
static const float GridHeight = g_winHeight * 1.0f / GridVerNum;
static const int LAYOUT_VERTEX_POS_DATA_FLOATS_NUM = (GridHorNum + 1) * (GridVerNum + 1) * 2; // pos is vec2 floats
static const int LAYOUT_VERTEX_IDX_DATA_NUM = GridHorNum * GridVerNum * 2 * 3; // triangles * 3 vertex index
static const int LAYOUT_RECT_NUM = GridVerNum * GridHorNum;
static int g_VRSPattern[GridVerNum][GridHorNum] = {{0}};

static GLfloat* g_pVertexPosData = nullptr;
static GLfloat* g_pVertexUvData = nullptr;
static GLuint* g_pVertexIndexData = nullptr;

static void genTargetTexture()
{
    glDeleteTextures(1, &g_glData.targetTexID);
    // generate render target texture;
    glGenTextures(1, &g_glData.targetTexID);
    glBindTexture(GL_TEXTURE_2D, g_glData.targetTexID);
    GLuint requestedFeatures = GL_FOVEATION_ENABLE_BIT_QCOM;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_FOVEATED_FEATURE_BITS_QCOM, requestedFeatures);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, g_winWidth, g_winHeight);
    glBindTexture(GL_TEXTURE_2D, 0);
    CHECK_GL_ERROR

    glGenRenderbuffers(1, &g_glData.depthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, g_glData.depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, g_winWidth, g_winHeight);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, g_glData.fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_glData.targetTexID, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, g_glData.depthRbo);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        ALOGE("FBO not complete");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CHECK_GL_ERROR
}

static void initGLResource()
{
    g_pVertexPosData = (GLfloat*) malloc(LAYOUT_VERTEX_POS_DATA_FLOATS_NUM * sizeof(GLfloat));
    g_pVertexUvData = (GLfloat*) malloc(LAYOUT_VERTEX_POS_DATA_FLOATS_NUM * sizeof(GLfloat));
    g_pVertexIndexData = (GLuint*) malloc(LAYOUT_VERTEX_IDX_DATA_NUM * sizeof(GLuint));
    const float interHor = 2.0f / GridHorNum;
    const float interVer = 2.0f / GridVerNum;
    for (int i = 0; i < (GridVerNum + 1); ++i) {
        for (int j = 0; j < (GridHorNum + 1); ++j) {
            int xPos = (i * (GridHorNum + 1) + j) * 2;
            int yPos = xPos + 1;
            g_pVertexPosData[xPos] = -1.0f + j * interHor;
            g_pVertexPosData[yPos] = -1.0f + i * interVer;
            g_pVertexUvData[xPos] = 0.0f + j * interHor / 2;
            g_pVertexUvData[yPos] = 0.0f + i * interVer / 2;
        }
    }

    for (int i = 0; i < GridVerNum; ++i) {
        for (int j = 0; j < GridHorNum; ++j) {
            int index = (i * GridHorNum + j) * 6;
            int base = i * (GridHorNum + 1) + j;
            g_pVertexIndexData[index] = base;
            g_pVertexIndexData[index + 1] = base + GridHorNum + 2;
            g_pVertexIndexData[index + 2] = base + GridHorNum + 1;
            g_pVertexIndexData[index + 3] = base;
            g_pVertexIndexData[index + 4] = base + 1;
            g_pVertexIndexData[index + 5] = base + GridHorNum + 2;
        }
    }

    g_glData.currentDisplay = eglGetCurrentDisplay();
    g_glData.currentContext = eglGetCurrentContext();
    if(g_glData.currentDisplay == EGL_NO_DISPLAY ||
       g_glData.currentContext == EGL_NO_CONTEXT)
    {
        ALOGE("No current EGLDisplay or EGLContext in Foveation module");
        return;
    }

    // generate fbo
    glGenFramebuffers(1, &g_glData.fbo);
    genTargetTexture();
    CHECK_GL_ERROR

    // generate vao and vbo
    glGenVertexArrays(1, &g_glData.vao);
    glGenBuffers(1, &g_glData.posVbo);
    glGenBuffers(1, &g_glData.uvVbo);
    glGenBuffers(1, &g_glData.ebo);
    glBindVertexArray(g_glData.vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_glData.posVbo);
    glBufferData(GL_ARRAY_BUFFER, LAYOUT_VERTEX_POS_DATA_FLOATS_NUM * sizeof(GLfloat), g_pVertexPosData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    glBindBuffer(GL_ARRAY_BUFFER, g_glData.uvVbo);
    glBufferData(GL_ARRAY_BUFFER, LAYOUT_VERTEX_POS_DATA_FLOATS_NUM * sizeof(GLfloat), g_pVertexUvData, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_glData.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, LAYOUT_VERTEX_IDX_DATA_NUM * sizeof(GLuint), g_pVertexIndexData, GL_STATIC_DRAW);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0); // VBO bind back to 0, need to be called after VAO back to 0
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
    CHECK_GL_ERROR

    // create program
    g_glData.program = glUtils::createProgram(g_vtxShaderSrc, g_frgShaderSrc);
    CHECK_GL_ERROR
}

static void renderOnFrame(int showTexture)
{
    CHECK_GL_ERROR
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, g_glData.fbo);CHECK_GL_ERROR
    glViewport(0, 0, g_winWidth, g_winHeight);CHECK_GL_ERROR
//    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);CHECK_GL_ERROR
//    glClear(GL_COLOR_BUFFER_BIT);CHECK_GL_ERROR

    glUseProgram(g_glData.program);
    CHECK_GL_ERROR
    glBindVertexArray(g_glData.vao);
    CHECK_GL_ERROR
    glUniform1i(glGetUniformLocation(g_glData.program, "srcTex"), 0);
    CHECK_GL_ERROR
    glUniform1i(glGetUniformLocation(g_glData.program, "showColor"), showTexture);
    CHECK_GL_ERROR
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_srcTexID);

    for (int i = 0; i < GridVerNum; ++i)
    {
        for (int j = 0; j < GridHorNum; ++j) {
            p_glShadingRateQCOM(g_VRSPattern[i][j]);
            int index = i * GridHorNum + j;
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(index * 6 * 4));
        }
    }

    p_glShadingRateQCOM(GL_SHADING_RATE_1X1_PIXELS_QCOM);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void grReleaseFoveationByVRS()
{
    glDeleteBuffers(1, &g_glData.uvVbo);
    glDeleteBuffers(1, &g_glData.posVbo);
    glDeleteBuffers(1, &g_glData.ebo);
    glDeleteVertexArrays(1, &g_glData.vao);
    glDeleteTextures(1, &g_glData.targetTexID);
    glDeleteFramebuffers(1, &g_glData.fbo);
    glDeleteProgram(g_glData.program);
    free(g_pVertexPosData);
    free(g_pVertexUvData);
    free(g_pVertexIndexData);
}

unsigned int grInitFoveationByVRS(unsigned int srcTexID,
                                 int targetWidth,
                                 int targetHeight)
{
    assert(srcTexID != 0);
    assert ((targetWidth  > 0) && (targetHeight > 0));
    g_winWidth = targetWidth;
    g_winHeight = targetHeight;
    initGLResource();

    // update src texture ID
    g_srcTexID = srcTexID;

    p_glShadingRateQCOM = (PFNGLSHADINGRATEQCOMPROC)eglGetProcAddress("glShadingRateQCOM");

    return g_glData.targetTexID;
}

void grProcessFoveationByVRS(void* pPara, int showTexture)
{
    FoveationParameters fovPara;
    memcpy(&fovPara, pPara, sizeof(FoveationParameters));
    CHECK_GL_ERROR
    for (int i = 0; i < GridVerNum; ++i) {
        for (int j = 0; j < GridHorNum; ++j) {
            int base = i * (GridHorNum + 1) + j;
            int bottomRight = base + 1;
            int topLeft = base + GridHorNum + 1;
            int topRight = base + GridHorNum + 2;
            double minX = g_pVertexPosData[base*2];
            double maxX = g_pVertexPosData[topRight*2];
            double disX = 0.0;
            if ((fovPara.focalX < minX) || (fovPara.focalX > maxX)) {
                disX = fmin(abs(fovPara.focalX - minX), abs(fovPara.focalX - maxX));
            }

            double minY = g_pVertexPosData[base*2+1];
            double maxY = g_pVertexPosData[topRight*2+1];
            double disY = 0.0;
            if ((fovPara.focalY < minY) || (fovPara.focalY > maxY)) {
                disY = fmin(abs(fovPara.focalY - minY), abs(fovPara.focalY - maxY));
            }

            double maxDensity = 1.0 / fmax(1.0, disX*disX*fovPara.gainX*fovPara.gainX + disY*disY*fovPara.gainY*fovPara.gainY - fovPara.fovArea);
            if (maxDensity > 0.5) {
                g_VRSPattern[i][j] = GL_SHADING_RATE_1X1_PIXELS_QCOM;
            } else if (maxDensity > 0.25) {
                g_VRSPattern[i][j] = GL_SHADING_RATE_2X1_PIXELS_QCOM;
            } else if (maxDensity > 0.125) {
                g_VRSPattern[i][j] = GL_SHADING_RATE_2X2_PIXELS_QCOM;
            } else if (maxDensity > 0.0625) {
                g_VRSPattern[i][j] = GL_SHADING_RATE_4X2_PIXELS_QCOM;
            } else {
                g_VRSPattern[i][j] = GL_SHADING_RATE_4X4_PIXELS_QCOM;
            }
        }
    }
    renderOnFrame(showTexture);
}

