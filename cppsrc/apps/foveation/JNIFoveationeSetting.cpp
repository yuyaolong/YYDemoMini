//
// Created by yuyao on 2022/6/13.
// Foveation + AAFT test
//
#include <android/log.h>
#include <GLES3/gl32.h>
#include <EGL/egl.h>
#include <string>
#include <fstream>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <cmath>
#include <vector>
#include "miscUtils.h"
#include "drawPoints.h"
#include "qcomFoveation.h"
#include "foveationByVRS.h"
#include "showOnScreenMultiview.h"
#include "showOnScreen.h"



static GLuint g_srcTexID = 0;
static GLuint g_foveationTargetTex = 0;
static GLuint g_AAFTConcatTargetTex = 0;
static int g_srcTexWidth = 0;
static int g_srcTexHeight = 0;
static int g_multiviewRenderTexWidth = 1920;
static int g_multiviewRenderTexHeight = 1920;
static int g_screenWidth = 0;
static int g_screenHeight = 0;

static int frameCnt = 0;
static double drawAvgTime = 0.0;

extern "C"
JNIEXPORT void JNICALL
Java_com_example_yuyaolong_yyalDemo_foveation_FoveationGLSurfaceRender_glNativeResize(JNIEnv *env,
                                        jobject thiz,
                                        jint width,
                                        jint height) {
    ALOGD("glNativeResize");
    g_screenWidth = width;
    g_screenHeight = height;
}

// ratio can be 2, 4, 8, 16
static void calculatePointsData(double ratio,
                                double focalX,
                                double focalY,
                                double fovArea,
                                double gainX,
                                double gainY,
                                int totalGrids,
                                std::vector<float>& data) {
    const int dataNum = (totalGrids + 1) * 4; // pos is vec2 floats
    double disHalfX = sqrt((ratio + fovArea) / gainX / gainX);
    double beginPx = focalX - disHalfX;
    double gridInter = 2.0f * disHalfX / totalGrids;
    for (int i = 0; i <= totalGrids; ++i) {
        float px = beginPx + i * gridInter;
        float tmp = ratio + fovArea - (px - focalX) * (px - focalX) * gainX * gainX;
        tmp = tmp < 0.0f ? 0.0f : tmp;
        float disHalfY = sqrt(tmp / gainY / gainY);
        data.push_back(px);
        data.push_back(focalY + disHalfY);
        data.push_back(px);
        data.push_back(focalY - disHalfY);
    }
}

static FoveationParameters g_fovPara[2];

int afterAAFTWidth = 3584;
int afterAAFTHeight = 1344;

extern "C"
JNIEXPORT void JNICALL
Java_com_example_yuyaolong_yyalDemo_foveation_FoveationGLSurfaceRender_glNativeInit(JNIEnv *env,
                                      jobject thiz,
                                      jobject assertManager) {
    EGLDisplay mainDisplay = eglGetCurrentDisplay();


    AAssetManager *nativeAsset = AAssetManager_fromJava(env, assertManager);
    AAsset *assetFile = AAssetManager_open(nativeAsset, "grid1920.jpg", AASSET_MODE_BUFFER);
    size_t bufferLen = AAsset_getLength(assetFile);

    const unsigned char *dataBuff = (const unsigned char *) AAsset_getBuffer(assetFile);
    g_srcTexID = MiscUtils::loadTextureByBuffer(dataBuff, bufferLen, &g_srcTexWidth, &g_srcTexHeight);
    AAsset_close(assetFile);

    g_foveationTargetTex = grInitFoveationQcom(g_srcTexID, g_multiviewRenderTexWidth, g_multiviewRenderTexHeight, YYALSettings::useSampledLayout);
    grInitDrawPoints(g_multiviewRenderTexWidth, g_multiviewRenderTexHeight);
    grInitShowOnScreenMultiview(YYALSettings::useSampledLayout);
    grInitShowOnScreen(nativeAsset);


    g_fovPara[0] = {3.0, 2.6458, 2.6458,0.0, 0.0};
    //g_fovPara[1] = {1.2, 6.1, 4.5,0.0, 0.0};
}

static int g_totalGridNum = 80;

extern "C"
JNIEXPORT void JNICALL
Java_com_example_yuyaolong_yyalDemo_foveation_FoveationGLSurfaceRender_glNativeRender(JNIEnv *env,
                                        jobject thiz, jint showTextureColor) {
    ALOGD("glNativeRender");

    std::vector<float> vertexData[2];
    double ratios[4] = {2.0, 4.0, 8.0, 16.0};
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 4; ++j) {
            calculatePointsData(ratios[j],
                                g_fovPara[i].focalX,
                                g_fovPara[i].focalY,
                                g_fovPara[i].fovArea,
                                g_fovPara[i].gainX,
                                g_fovPara[i].gainY,
                                g_totalGridNum,
                                vertexData[i]);
        }
    }
    ++frameCnt;
    int64_t t1 = glUtils::getTimeNsec();
    grProcessFoveationQcom(g_fovPara, 1, 1);
    grProcessShowOnScreen(g_srcTexID, 0, 0, g_screenWidth, g_screenHeight/2);
    grProcessShowOnScreenMultiview(g_foveationTargetTex, 0, 0, g_screenHeight/2, g_screenWidth, g_screenHeight/2, false);
    grProcessDrawPointsOnScreen(vertexData[0].data(), vertexData[0].size(), 0.0f, 0.0f, 0.0f,
                                0, g_screenHeight/2, g_screenWidth/2, g_screenHeight/2);
    glFinish();

    int64_t t2 = glUtils::getTimeNsec();
    double drawTime = (t2 - t1) / 1000000.0;
    //dumpTex(g_foveationTargetTex, g_srcTexWidth, g_srcTexHeight, 1);
    drawAvgTime = (drawAvgTime * (frameCnt - 1) + drawTime) / frameCnt;
    ALOGD("yyal avg draw time: %f ms", drawAvgTime);
    ALOGD("yyal avg draw frame cnt: %d", frameCnt);

}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_yuyaolong_yyalDemo_foveation_FoveationGLSurfaceRender_glDestroy(JNIEnv *env,
                                   jobject thiz) {
    grReleaseFoveationQcom();
    grReleaseDrawPoints();
    grReleaseShowOnScreenMultiview();
    grReleaseShowOnScreen();
}

static double g_inter = 0.1;
static double g_f_inter = 0.005;

static int g_focalIndex = 0;

extern "C"
JNIEXPORT void JNICALL
Java_com_example_yuyaolong_yyalDemo_foveation_FoveationGLSurfaceRender_glUpdateFoveation(
        JNIEnv *env, jobject thiz, jint msg) {
    if (msg == 1) {
        g_fovPara[g_focalIndex].fovArea += g_inter;
    } else if (msg == 2) {
        g_fovPara[g_focalIndex].fovArea -= g_inter;
    } else if (msg == 3) {
        g_fovPara[g_focalIndex].gainX += g_inter;
    } else if (msg == 4) {
        g_fovPara[g_focalIndex].gainX -= g_inter;
    } else if (msg == 5) {
        g_fovPara[g_focalIndex].gainY += g_inter;
    } else if (msg == 6) {
        g_fovPara[g_focalIndex].gainY -= g_inter;
    } else if (msg == 7) {
        g_fovPara[g_focalIndex].focalX += g_f_inter;
    } else if (msg == 8) {
        g_fovPara[g_focalIndex].focalX -= g_f_inter;
    } else if (msg == 9) {
        g_fovPara[g_focalIndex].focalY += g_f_inter;
    } else if (msg == 10) {
        g_fovPara[g_focalIndex].focalY -= g_f_inter;
    } else if (msg == 11) {
        g_focalIndex = 0;
    } else if (msg == 12) {
        g_focalIndex = 1;
    } else {
        ALOGE("update foveation error");
    }
    ALOGD("f%d g_fovArea: %f, g_gainX: %f, g_gainY: %f, g_focalX: %f, g_focalY: %f",
          g_focalIndex,
          g_fovPara[g_focalIndex].fovArea,
          g_fovPara[g_focalIndex].gainX,
          g_fovPara[g_focalIndex].gainY,
          g_fovPara[g_focalIndex].focalX,
          g_fovPara[g_focalIndex].focalY);
}