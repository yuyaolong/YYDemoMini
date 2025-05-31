//
// Created by yuyaolong on 2018/8/4.
//

#include <jni.h>
#include <ctime>
#include <string>
#include <fstream>
#include <pthread.h>
#include <unistd.h>
#include "miscUtils.h"
#include "models.h"
#include "GLCamera.h"
#include "showOnScreen.h"
#include "equirToCubemap.h"
#include "pbrRender.h"
#include "environmentMap.h"
#include "irradianceMap.h"
#include "SH9Extract.h"
#include "specularPrefilterMap.h"
#include "specularBRDFIntegrationMap.h"

namespace {
    AAssetManager* g_nativeAsset = nullptr;

    GLuint g_hdrEnvTexID = 0;

    int g_screenWidth = 0;
    int g_screenHeight = 0;

    GLCamera* g_pGLCamera = nullptr;

    GLuint g_envCubeMapTexID = 0;
    GLuint g_irradianceCubeMapTexID = 0;
    GLuint g_specularPrefilterMapTexID = 0;
    GLuint g_specularBRDFIntegrateMapTexID = 0;

    GLuint g_onScreenTexID = 0;

    GLuint g_environmentMapRbo = 0;
    GLuint g_specularPrefilterMapRbo = 0; // generated in module internal
    GLuint g_specularBRDFIntegrateMapRbo = 0;
    GLuint g_PBRRbo = 0;
}

extern "C" {
JNIEXPORT void JNICALL
Java_com_example_yuyaolong_yyalDemo_lightCube_IBLTestInterface_cubeInit(JNIEnv *env,
                         jobject instance,
                         jobject assertManager) {
    g_pGLCamera = new GLCamera();
    g_nativeAsset = AAssetManager_fromJava(env, assertManager);
    EGLDisplay mainDisplay = eglGetCurrentDisplay();
    eglSwapInterval(mainDisplay, 0);

    int texWidth = 0;
    int texHeight = 0;
    g_hdrEnvTexID = glUtils::loadHDREnvTextureFromAssetFile(g_nativeAsset, "HDR/Arches_E_PineTree_3k.hdr", texWidth, texHeight);


    // set camera parameters
    glm::vec3 cameraPos(0, 0, 9);
    glm::vec3 cameraLookAtPoint(0, 0, 0);
    glm::vec3 cameraLookUp(0, 1, 0);
    g_pGLCamera->cameraLookAt(cameraPos, cameraLookAtPoint, cameraLookUp);
    g_pGLCamera->objectScale(2.0f);

    /* --------------------- generate target textures -------------------------------*/
    // create environmap Texture
    // environmentMapSize
    int envMapWidth= 512;
    int envMapHeight= 512;
    GLuint equirToCubeMapRbo = glUtils::createDepthStencilRbo(envMapWidth, envMapHeight, GL_DEPTH_COMPONENT24);
    g_envCubeMapTexID = glUtils::createTexture(envMapWidth,
                                            envMapHeight,
                                            GL_TEXTURE_CUBE_MAP,
                                            GL_RGB16F,
                                            GL_RGB,
                                            1, 1,
                                            GL_FLOAT,nullptr,
                                            GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, // will gen mipmap
                                            GL_CLAMP_TO_EDGE,
                                            false, false);
    // irradianceMap Size
    int irradianceCubeMapRboWidth= 32;
    int irradianceCubeMapRboHeight= 32;
    GLuint irradianceCubeMapRbo = glUtils::createDepthStencilRbo(irradianceCubeMapRboWidth, irradianceCubeMapRboHeight, GL_DEPTH_COMPONENT24);
    g_irradianceCubeMapTexID = glUtils::createTexture(irradianceCubeMapRboWidth,
                                                      irradianceCubeMapRboHeight,
                                                      GL_TEXTURE_CUBE_MAP,
                                                      GL_RGB16F,
                                                      GL_RGB,
                                                      1, 1,
                                                      GL_FLOAT,nullptr,
                                                      GL_LINEAR, GL_LINEAR,
                                                      GL_CLAMP_TO_EDGE,
                                                      false, false);

    int specularPrefilterMapWidth = 128;
    int specularPrefilterMapHeight = 128;
    g_specularPrefilterMapTexID = glUtils::createTexture(specularPrefilterMapWidth,
                                                    specularPrefilterMapHeight,
                                                    GL_TEXTURE_CUBE_MAP,
                                                    GL_RGB16F,
                                                    GL_RGB,
                                                    1, 1,
                                                    GL_FLOAT,nullptr,
                                                    GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, // will gen mipmap
                                                    GL_CLAMP_TO_EDGE,
                                                    false, false);
    // prepare for as render target and PBR source
    glBindTexture(GL_TEXTURE_CUBE_MAP, g_specularPrefilterMapTexID);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    glGenRenderbuffers(1, &g_specularPrefilterMapRbo); // buffer container generated in process

    int specularBRDFIntegrateMapWidth = 512;
    int specularBRDFIntegrateMapHeight = 512;
    g_specularBRDFIntegrateMapTexID = glUtils::createTexture(specularBRDFIntegrateMapWidth,
                                                             specularBRDFIntegrateMapHeight,
                                                             GL_TEXTURE_2D,
                                                             GL_RG16F, GL_RG,
                                                             1,1,
                                                             GL_FLOAT, nullptr,
                                                             GL_LINEAR, GL_LINEAR,
                                                             GL_CLAMP_TO_EDGE,
                                                             true, false);
    g_specularBRDFIntegrateMapRbo = glUtils::createDepthStencilRbo(specularBRDFIntegrateMapWidth,
                                                                   specularBRDFIntegrateMapHeight,
                                                                   GL_DEPTH_COMPONENT24);

    /* --------------------- init modules and process once operation  -------------------------------*/
    grInitEquirToCubemap(g_nativeAsset);
    grInitIrradianceMap(g_nativeAsset);
    grProcessEquirToCubemap(g_hdrEnvTexID,
                             g_envCubeMapTexID,
                             equirToCubeMapRbo,
                             envMapWidth,
                             envMapHeight);

    glBindTexture(GL_TEXTURE_CUBE_MAP, g_envCubeMapTexID);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    grProcessIrradianceMap(g_envCubeMapTexID,
                           g_irradianceCubeMapTexID,
                           irradianceCubeMapRbo,
                           irradianceCubeMapRboWidth,
                           irradianceCubeMapRboHeight);

    // extract SH9 from irradiance map
    ShChannel SH9_RGB[3];
    memset(&SH9_RGB[0], 0, sizeof(SH9_RGB));
    float SH9_vec4s[9 * 4] = {0.0f};
    grProcessSH9Extract(g_irradianceCubeMapTexID, 32, 32, 3, SH9_RGB);
    for (int i = 0; i < 9; ++i) {
        ALOGM("vec3(%f, %f, %f);", float(SH9_RGB[0].coeffs[i]), float(SH9_RGB[1].coeffs[i]), float(SH9_RGB[2].coeffs[i]));
        SH9_vec4s[i*4 + 0] = float(SH9_RGB[0].coeffs[i]);
        SH9_vec4s[i*4 + 1] = float(SH9_RGB[1].coeffs[i]);
        SH9_vec4s[i*4 + 2] = float(SH9_RGB[2].coeffs[i]);
        SH9_vec4s[i*4 + 3] = 0.0f;
    }



    grInitEnvironmentMap(g_nativeAsset);
    int attributesSizeArray[3] = {3, 3, 2};
//    const char* folderName = "granite";
    const char* folderName = "rustediron";
//    const char* folderName = "streakedMetal";

    std::vector<float> sphereVboData;
    std::vector<unsigned int> sphereEboData;
    glUtils::generateSphereVboAndEboData(sphereVboData, sphereEboData);


    /*-------------------------------------- IBL specular init part----------------------------*/
    grInitSpecularPrefilterMap(g_nativeAsset);
    grProcessSpecularPrefilterMap(g_envCubeMapTexID,
                                  g_specularPrefilterMapTexID,
                                  g_specularPrefilterMapRbo,
                                  specularPrefilterMapWidth,
                                  specularPrefilterMapHeight);


    grInitSpecularBRDFIntegrationMap(g_nativeAsset);
    grProcessSpecularBRDFIntegrationMap(specularBRDFIntegrateMapWidth,
                                        specularBRDFIntegrateMapHeight,
                                        g_specularBRDFIntegrateMapTexID,
                                        g_specularBRDFIntegrateMapRbo);

    /*-------------------------------------- PBR init ----------------------------------------*/

    grInitPbrRender(g_nativeAsset,
                    sphereVboData.data(),
                    sphereVboData.size(),
                    sphereEboData.data(),
                    sphereEboData.size(),
                    GL_TRIANGLE_STRIP,
                    attributesSizeArray,
                    3,
                    sphereEboData.size(),
                    folderName,
                    SH9_vec4s);
    grInitShowOnScreen(g_nativeAsset);

}


JNIEXPORT void JNICALL
Java_com_example_yuyaolong_yyalDemo_lightCube_IBLTestInterface_cubeRender(JNIEnv *env, jobject instance) {


    g_pGLCamera->calculateMVPMat4();
    glm::mat4 modelMat4 = g_pGLCamera->getModelMat4();
    glm::mat4 viewMat4 = g_pGLCamera->getViewMat4();
    glm::mat4 projectionMat4 = g_pGLCamera->getProjectionMat4();
    glm::vec3 viewPos = g_pGLCamera->getCameraPosition();

    grProcessEnvironmentMap(g_envCubeMapTexID,
                            g_screenWidth,
                            g_screenHeight,
                            g_onScreenTexID,
                            0,
                            g_environmentMapRbo,
                            &viewMat4,
                            &projectionMat4);

    grProcessPbrRender(g_irradianceCubeMapTexID,
                        g_specularPrefilterMapTexID,
                        g_specularBRDFIntegrateMapTexID,
                        g_screenWidth,
                        g_screenHeight,
                        g_onScreenTexID,
                        0,
                        g_PBRRbo,
                         &modelMat4,
                         &viewMat4,
                         &projectionMat4,
                         &viewPos,
                         false);

    grProcessShowOnScreen(g_onScreenTexID, 0, 0, g_screenWidth, g_screenHeight);
}

JNIEXPORT void JNICALL
Java_com_example_yuyaolong_yyalDemo_lightCube_IBLTestInterface_cubeResize(JNIEnv *env,
                               jobject instance,
                               jint width,
                               jint height) {
    float aspect = (GLfloat) width / (GLfloat) height;
    g_pGLCamera->setAspect(aspect);


    if ( (g_screenWidth != width) ||
         (g_screenHeight != height)) {
        glDeleteRenderbuffers(1, &g_environmentMapRbo);
        g_environmentMapRbo = glUtils::createDepthStencilRbo(width, height, GL_DEPTH_COMPONENT24);
        glDeleteRenderbuffers(1, &g_PBRRbo);
        g_PBRRbo = glUtils::createDepthStencilRbo(width, height, GL_DEPTH_COMPONENT24);

        glDeleteTextures(1, &g_onScreenTexID);
        g_onScreenTexID = glUtils::createTexture(width, height);
    }

    g_screenWidth = width;
    g_screenHeight = height;
    CHECK_GL_ERROR
}

JNIEXPORT void JNICALL
Java_com_example_yuyaolong_yyalDemo_lightCube_IBLTestInterface_cubeClean(JNIEnv *env, jobject instance) {

    ALOGD("hehe free camera");
    delete g_pGLCamera;
    g_pGLCamera = nullptr;
    grReleasePbrRender();
    grReleaseShowOnScreen();
    grReleaseEquirToCubemap();
    grReleaseEnvironmentMap();
    grReleaseIrradianceMap();
    grReleaseSpecularPrefilterMap();
    grReleaseSpecularBRDFIntegrationMap();
    glDeleteTextures(1, &g_hdrEnvTexID);
    glDeleteTextures(1, &g_envCubeMapTexID);
    glDeleteTextures(1, &g_irradianceCubeMapTexID);
    glDeleteTextures(1, &g_specularPrefilterMapTexID);
    glDeleteTextures(1, &g_onScreenTexID);
    glDeleteRenderbuffers(1, &g_PBRRbo);
    glDeleteRenderbuffers(1, &g_environmentMapRbo);
    glDeleteRenderbuffers(1, &g_specularPrefilterMapRbo);
}


JNIEXPORT void JNICALL Java_com_example_yuyaolong_yyalDemo_lightCube_IBLTestInterface_cubeScaleNative(
        JNIEnv *env,
        jobject instance,
        jfloat scaleFactor) {
    g_pGLCamera->objectScale(scaleFactor);
}

JNIEXPORT void JNICALL Java_com_example_yuyaolong_yyalDemo_lightCube_IBLTestInterface_cubeScrollNative(
        JNIEnv *env,
        jobject instance,
        jfloat dx,
        jfloat dy,
        jfloat endPx,
        jfloat endPy) {
    // origin of screen resolution coordinate is at left-top, move origin to center,
    // set up is y positive, right is x positive, range (-1, 1)
    float distanceX = (float) dx / g_screenWidth;
    float distanceY = -(float) dy / g_screenHeight;
    float endPosX = 2 * endPx / g_screenWidth - 1.0f;
    float endPosY = -2 * endPy / g_screenHeight + 1.0f;

    //ALOGD("size Width: %d, Height: %d", g_screenWidth, g_screenHeight);

    g_pGLCamera->objectFingerRotate(distanceX, distanceY, endPosX, endPosY);
}

JNIEXPORT void JNICALL
Java_com_example_yuyaolong_yyalDemo_lightCube_IBLTestInterface_cameraRotateNative(JNIEnv *env,
                                     jobject thiz,
                                     jfloat dx,
                                     jfloat dy,
                                     jfloat endPx,
                                     jfloat endPy) {
    // origin of screen resolution coordinate is at left-top, move origin to center,
    // set up is y positive, right is x positive, range (-1, 1)
    float distanceX = (float) dx / g_screenWidth;
    float distanceY = -(float) dy / g_screenHeight;
    float endPosX = 2 * endPx / g_screenWidth - 1.0f;
    float endPosY = -2 * endPy / g_screenHeight + 1.0f;

    g_pGLCamera->cameraRotate(distanceX, distanceY, endPosX, endPosY);
}

JNIEXPORT void JNICALL
Java_com_example_yuyaolong_yyalDemo_lightCube_IBLTestInterface_cameraScaleNative(JNIEnv *env,
                                                                                    jobject thiz,
                                                                                    jfloat dx,
                                                                                    jfloat dy,
                                                                                    jfloat endPx,
                                                                                    jfloat endPy) {
    // origin of screen resolution coordinate is at left-top, move origin to center,
    // set up is y positive, right is x positive, range (-1, 1)
    float distanceX = (float) dx / g_screenWidth;
    float distanceY = -(float) dy / g_screenHeight;
    float endPosX = 2 * endPx / g_screenWidth - 1.0f;
    float endPosY = -2 * endPy / g_screenHeight + 1.0f;

    g_pGLCamera->cameraScale(distanceX, distanceY, endPosX, endPosY);
}

JNIEXPORT void JNICALL
Java_com_example_yuyaolong_yyalDemo_lightCube_IBLTestInterface_cubeResetNative(JNIEnv *env, jobject instance) {
    g_pGLCamera->resetCamera();
}


JNIEXPORT void JNICALL
Java_com_example_yuyaolong_yyalDemo_lightCube_IBLTestInterface_cubeEnableHDRNative(JNIEnv *env,
                                     jobject instance,
                                     jboolean enableHDR) {
}

JNIEXPORT void JNICALL
Java_com_example_yuyaolong_yyalDemo_lightCube_IBLTestInterface_cubeUpdateHDRExposureNative(JNIEnv *env,
                                             jobject instance,
                                             jfloat exposure) {
}

JNIEXPORT void JNICALL
Java_com_example_yuyaolong_yyalDemo_lightCube_IBLTestInterface_cubeUpdateLightValueNative(JNIEnv *env,
                                            jobject instance,
                                            jfloat lightValue) {
}

JNIEXPORT void JNICALL
Java_com_example_yuyaolong_yyalDemo_lightCube_IBLTestInterface_cubeToggleVRSNative(JNIEnv *env,
                                     jobject thiz,
                                     jboolean enable_vrs) {
}

}
