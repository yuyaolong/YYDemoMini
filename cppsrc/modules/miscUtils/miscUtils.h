//
// Created by yaolo on 2022/8/18.
//
#ifndef MISCUTILS_H
#define MISCUTILS_H

#include <android/log.h>
#include <EGL/egl.h>
#include <GLES3/gl32.h>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <cstring>
#include <cassert>
#include <unistd.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>
#include <memory>
#include <android/asset_manager_jni.h>
#include "commonLog.h"

#if defined(__CYGWIN32__)
#define GRAVITYXR_RUNTIME_API __stdcall
	#define GRAVITYXR_RUNTIME_EXPORT __declspec(dllexport)
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WINAPI_FAMILY)
#define GRAVITYXR_RUNTIME_API __stdcall
	#define GRAVITYXR_RUNTIME_EXPORT __declspec(dllexport)
#elif defined(__MACH__) || defined(__ANDROID__) || defined(__linux__) || defined(__QNX__)
#define GRAVITYXR_RUNTIME_API
#define GRAVITYXR_RUNTIME_EXPORT
#else
#define GRAVITYXR_RUNTIME_API
	#define GRAVITYXR_RUNTIME_EXPORT
#endif


#if defined(GR_USE_MOCK_AAFT_ARGB) || defined(GR_USE_AAFT_ARGB)
#define GRTOnScreenSurfaceWidth 3584
#else
#define GRTOnScreenSurfaceWidth 2688
#endif
#define GRTOnScreenSurfaceHeight 1344

#if defined(GR_USE_MOCK_AAFT_SLAM_BLEND)
#define GRTClientBufferWidth 640
#define GRTClientBufferHeight 480
#else
#define GRTClientBufferWidth 1920
    #define GRTClientBufferHeight 1920
#endif

#define STR(s) #s
#define STRV(s) STR(s)

#define QCOM_DEVICE 1

#ifdef QCOM_DEVICE
typedef void (*PFNGLSHADINGRATEQCOMPROC)(int);
#define GL_SHADING_RATE_1X1_PIXELS_QCOM 0x96A6
#define GL_SHADING_RATE_1X2_PIXELS_QCOM 0x96A7
#define GL_SHADING_RATE_2X1_PIXELS_QCOM 0x96A8
#define GL_SHADING_RATE_2X2_PIXELS_QCOM 0x96A9
#define GL_SHADING_RATE_4X2_PIXELS_QCOM 0x96AC
#define GL_SHADING_RATE_4X4_PIXELS_QCOM 0x96AE
static PFNGLSHADINGRATEQCOMPROC p_glShadingRateQCOM = nullptr;

typedef void (*PFNGLTextureFoveationParametersQCOM)(unsigned int texture,
                                                    unsigned int layer,
                                                    unsigned int focalPoint,
                                                    float focalX,
                                                    float focalY,
                                                    float gainX,
                                                    float gainY,
                                                    float foveaArea);
static PFNGLTextureFoveationParametersQCOM p_glTextureFoveationParametersQCOM = nullptr;
#define GL_FOVEATION_ENABLE_BIT_QCOM 0x1
#define GL_FOVEATION_SCALED_BIN_METHOD_BIT_QCOM 0x2
#define GL_FOVEATION_SUBSAMPLED_LAYOUT_METHOD_BIT_QCOM 0x4
#define GL_TEXTURE_FOVEATED_FEATURE_BITS_QCOM 0x8BFB
#define GL_TEXTURE_FOVEATED_MIN_PIXEL_DENSITY_QCOM 0x8BFC
#define GL_TEXTURE_FOVEATED_FEATURE_QUERY_QCOM 0x8BFD
#define GL_TEXTURE_FOVEATED_NUM_FOCAL_POINTS_QUERY_QCOM 0x8BFE
#define GL_FRAMEBUFFER_INCOMPLETE_FOVEATION_QCOM 0x8BFE
#endif

typedef void* EGLImageKHR;
typedef EGLImageKHR (*PFNEGLCREATEIMAGEKHR)(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list);
static PFNEGLCREATEIMAGEKHR p_eglCreateImageKHR = nullptr;

typedef EGLClientBuffer (*PFNEGLGETNATIVECLIENTBUFFERANDROIDPROC)(const struct AHardwareBuffer * buffer);
static PFNEGLGETNATIVECLIENTBUFFERANDROIDPROC p_eglGetNativeClientBufferANDROID = nullptr;

#define EGL_IMAGE_PRESERVED_KHR 0x30D2
#define EGL_NATIVE_BUFFER_ANDROID 0x3140
#define EGL_NO_IMAGE_KHR ((EGLImageKHR)0)

typedef void* GLeglImageOES;
typedef void (*PFNEGLIMAGETARGETTEXTURE2DOES)(GLenum target, GLeglImageOES image);
static PFNEGLIMAGETARGETTEXTURE2DOES p_glEGLImageTargetTexture2DOES = nullptr;

enum bitfield {
    GL_COLOR_BUFFER_BIT0_QCOM                     = 0x00000001,
    GL_COLOR_BUFFER_BIT1_QCOM                     = 0x00000002,
    GL_COLOR_BUFFER_BIT2_QCOM                     = 0x00000004,
    GL_COLOR_BUFFER_BIT3_QCOM                     = 0x00000008,
    GL_COLOR_BUFFER_BIT4_QCOM                     = 0x00000010,
    GL_COLOR_BUFFER_BIT5_QCOM                     = 0x00000020,
    GL_COLOR_BUFFER_BIT6_QCOM                     = 0x00000040,
    GL_COLOR_BUFFER_BIT7_QCOM                     = 0x00000080,
    GL_DEPTH_BUFFER_BIT0_QCOM                     = 0x00000100,
    GL_DEPTH_BUFFER_BIT1_QCOM                     = 0x00000200,
    GL_DEPTH_BUFFER_BIT2_QCOM                     = 0x00000400,
    GL_DEPTH_BUFFER_BIT3_QCOM                     = 0x00000800,
    GL_DEPTH_BUFFER_BIT4_QCOM                     = 0x00001000,
    GL_DEPTH_BUFFER_BIT5_QCOM                     = 0x00002000,
    GL_DEPTH_BUFFER_BIT6_QCOM                     = 0x00004000,
    GL_DEPTH_BUFFER_BIT7_QCOM                     = 0x00008000,
    GL_STENCIL_BUFFER_BIT0_QCOM                   = 0x00010000,
    GL_STENCIL_BUFFER_BIT1_QCOM                   = 0x00020000,
    GL_STENCIL_BUFFER_BIT2_QCOM                   = 0x00040000,
    GL_STENCIL_BUFFER_BIT3_QCOM                   = 0x00080000,
    GL_STENCIL_BUFFER_BIT4_QCOM                   = 0x00100000,
    GL_STENCIL_BUFFER_BIT5_QCOM                   = 0x00200000,
    GL_STENCIL_BUFFER_BIT6_QCOM                   = 0x00400000,
    GL_STENCIL_BUFFER_BIT7_QCOM                   = 0x00800000,
    GL_MULTISAMPLE_BUFFER_BIT0_QCOM               = 0x01000000,
    GL_MULTISAMPLE_BUFFER_BIT1_QCOM               = 0x02000000,
    GL_MULTISAMPLE_BUFFER_BIT2_QCOM               = 0x04000000,
    GL_MULTISAMPLE_BUFFER_BIT3_QCOM               = 0x08000000,
    GL_MULTISAMPLE_BUFFER_BIT4_QCOM               = 0x10000000,
    GL_MULTISAMPLE_BUFFER_BIT5_QCOM               = 0x20000000,
    GL_MULTISAMPLE_BUFFER_BIT6_QCOM               = 0x40000000,
    GL_MULTISAMPLE_BUFFER_BIT7_QCOM               = 0x80000000
};
typedef void (*PFNGLSTARTTILINGQCOM)(uint x, uint y, uint width, uint height, bitfield preserveMask);

typedef void (*PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVR)(GLenum target, GLenum attachment,
                                                    GLuint texture, GLint level,
                                                    GLint baseViewIndex, GLsizei numViews);

typedef void (*PFNGLFRAMEBUFFERTEXTUREMULTISAMPLEMULTIVIEWOVR)(GLenum target, GLenum attachment,
                                                               GLuint texture, GLint level, GLsizei samples,
                                                               GLint baseViewIndex, GLsizei numViews);

typedef void (*PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEIMG)(GLenum target, GLenum attachment,
                                                        GLenum textarget, GLuint texture,
                                                        GLint level, GLsizei samples);

typedef void(*PFNGLRENDERBUFFERSTORAGEMULTISAMPLEIMG)(GLenum target, GLsizei samples,
                                                      GLenum internalformat,
                                                      GLsizei width, GLsizei height);


#define CHECK_GL_ERROR glUtils::checkGlError(__FILE__, __LINE__);
#define CHECK_EGL_ERROR glUtils::checkEGLError(__FILE__, __LINE__);
#define YYDEMO_ABORT glUtils::grAbort(__FILE__, __LINE__);
#define YY_DEMO_ASSERT(cond) MiscUtils::YYDemoAssert(cond, __FILE__, __LINE__);

namespace {
    typedef struct _foveationParamters {
        double fovArea;
        double gainX;
        double gainY;
        double focalX;
        double focalY;
    } FoveationParameters;

    PFNGLSTARTTILINGQCOM p_glStartTilingQCOM = nullptr;
    PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVR p_glFramebufferTextureMultiviewOVR = nullptr;
    PFNGLFRAMEBUFFERTEXTUREMULTISAMPLEMULTIVIEWOVR p_glFramebufferTextureMultisampleMultiviewOVR = nullptr;
    PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEIMG g_glFramebufferTexture2DMultisampleEXT = nullptr;
    PFNGLRENDERBUFFERSTORAGEMULTISAMPLEIMG g_glRenderbufferStorageMultisampleEXT = nullptr;
}

#define IMAGE_FORMAT_RGBA           0x01
#define IMAGE_FORMAT_NV21           0x02
#define IMAGE_FORMAT_NV12           0x03
#define IMAGE_FORMAT_I420           0x04
#define IMAGE_FORMAT_RGB            0x05
#define IMAGE_FORMAT_I420A          0x06

#define IMAGE_FORMAT_RGBA_EXT       "RGB32"
#define IMAGE_FORMAT_NV21_EXT       "NV21"
#define IMAGE_FORMAT_NV12_EXT       "NV12"
#define IMAGE_FORMAT_I420_EXT       "I420"
#define IMAGE_FORMAT_RGB_EXT        "RGB24"
#define IMAGE_FORMAT_I420A_EXT      "I420A"

#define GL_TEXTURE_MAX_ANISOTROPY_EXT          0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT      0x84FF

enum YYDemoRet {
    YYDemoRetSuccess = 0,
    YYDemoRetError = 1
};

namespace MiscUtils {
    typedef struct _tag_NativeImage {
        int width;
        int height;
        int format;
        char *ppPlane[4];

        _tag_NativeImage() {
            width = 0;
            height = 0;
            format = 0;
            ppPlane[0] = nullptr;
            ppPlane[1] = nullptr;
            ppPlane[2] = nullptr;
            ppPlane[3] = nullptr;
        }
    } NativeImage;

    void DumpNativeImage(NativeImage *pSrcImg, const char *pPath, const char *pFileName, int cnt) ;
    void dumpFBO0Color(int width, int height, int flipY, int frameCnt, const char* name);
    void dumpCurrentFBOColor(int width, int height, int flipY, int frameCnt, const char* name);
    void dumpTex(unsigned int tId, int width, int height, int flipY, int frameCnt, const char* name);
    void dumpPNG(int width, int height, const void *pData, int flipY, int frameCnt, const char* name);
    void dumpNamePNG(int width, int height, int comp, const void *pData, int flipY, const char* name);
    unsigned int loadTextureByBuffer(const unsigned char *dataBuffer, int bufferLen, int* pTexWidth, int* pTexHeight);
    void readPixels (GLuint texID, void* bufferPtr, const int width, const int height);
    int grtCheckError(int error, const char* errorMsg);
    void* justStbImageLoadFromMemory(const unsigned char *dataBuffer, int bufferLen, int* pTexWidth, int* pTexHeight, int* channels);
    void justStbFreeDataBuffer(void* imageBuffer);
    void extractRawAssetData(void*nativeAsset, const char* fileName, std::vector<float>& floatData);
    unsigned int loadTextureArrayByBuffers(const std::vector<float>* floatDataArray, int width, int height, int depth,
                                           GLenum internalFormat, GLenum format, GLenum dataType);
    void getShaderStringFromAsset(AAssetManager* assetMgr, const char* fileName, std::string& outString);
    long getCurrentTimeInSeconds();

    void getV4RDataFromFile(AAssetManager* assetMgr, const char* fileName, std::vector<float>& outVec);
    void getGazeDataFromFile(AAssetManager* assetMgr, const char* fileName, std::vector<float>& outVec);

    void YYDemoAssert(bool cond, const char* fileName, int lineNum);
}

namespace glUtils {
    unsigned int loadTextureFromAssetFile(void* nativeAsset, const char* filename, int* width, int* height);
    unsigned int loadHDREnvTextureFromAssetFile(void* nativeAsset, const char* filename, int& width, int& height);
    int64_t getTimeNsec();
    double showTimeDiff(int64_t t1, int64_t t2, double drawAvgTime, int frameCnt, const char* msg);
    void showTimeDiffAbs(int64_t t1, int64_t t2, const char* msg);
    void checkEGlError(const char* fileName, int lineNum);
    void checkGlError(const char* fileName, int lineNum);
    void grAbort(const char* fileName, int lineNum);
    GLuint hardwareBufferToTexture(AHardwareBuffer* buffer, GLint texType, GLint filter, EGLImage* retEGLImage, bool useSRGB=false);
    void deleteEGLResources(GLuint texID, EGLImage image);
    GLuint genTexture(int texWidth, int texHeight);
    const char* getShaderString(GLenum shaderType);
    GLuint createShader(GLenum shaderType, const char *src);
    GLuint createProgram(const char *vtxSrc, const char *fragSrc);
    GLuint createProgramFromAssetName(void* nativeAssets, const char *vsFileName, const char *fsFileName);
    GLuint createTexture(int width,
                         int height,
                         GLenum textureTarget = GL_TEXTURE_2D,
                         GLenum internalFormat = GL_RGBA8,
                         GLenum format = GL_RGBA,
                         int depth = 1,
                         int mipmapLevels = 1,
                         GLenum type = GL_UNSIGNED_BYTE,
                         void* data = nullptr,
                         GLint minFilter = GL_LINEAR,
                         GLint maxFilter = GL_LINEAR,
                         GLint wrapWay = GL_CLAMP_TO_EDGE,
                         bool immutable = true,
                         bool anisotropy = false);
    GLuint createDepthStencilRbo (int width, int height, GLenum internalFormat);
    GLuint genAnisotropyTexture(int texWidth, int texHeight);
    void enableColorTexFoveation(GLenum texType, GLuint texID, bool enableSubSampleLayout, float gazeH, float gazeV);

    void initAllFuncAddress();

    // utility uniform functions
    void setUniformBool(GLuint program, const char* name, bool value);
    void setUniformInt(GLuint program, const char* name, int value);
    void setUniformFloat(GLuint program, const char* name, float value);
    void setUniformVec2(GLuint program, const char* name, const float* ptr);
    void setUniformVec2(GLuint program, const char* name, float x, float y);
    void setUniformVec3(GLuint program, const char* name, const float* ptr);
    void setUniformVec3(GLuint program, const char* name, float x, float y, float z);
    void setUniformVec4(GLuint program, const char* name, const float* ptr);
    void setUniformVec4(GLuint program, const char* name, float x, float y, float z, float w);
    void setUniformMat2(GLuint program, const char* name, const float* ptr);
    void setUniformMat3(GLuint program, const char* name, const float* ptr);
    void setUniformMat4(GLuint program, const char* name, const float* ptr);

    // GL_TRIANGLE_STRIP
    // pos3 normal3 uv2 -- vbo
    void generateSphereVboAndEboData(std::vector<float>& vboData, std::vector<unsigned int>& eboData);
}

namespace YYALSettings {
    bool useFoveationRender = true;
    bool useSampledLayout = false;
    bool useSRGB = false;
    bool showFoveationRender = true;
    bool dumpColor = false;

    const float aaftTest_TileSize = 32.0;
    const int aaftTest_multiviewTargetWidth = 3840; // 1920 or 3840
    const int aaftTest_multiviewTargetHeight = 3840; // 1920 or 3840
    const int aaftTest_TargetWidth = 4224; // 2688 or 4400
    const int aaftTest_TargetHeight = 2112; // 1344 or 2200
}

#define STR(s) #s
#define STRV(s) STR(s)

#define GAZE_DATA_SET 3840_2112_32_real

#endif //ANDROIDVRS_MISCUTILS_H
