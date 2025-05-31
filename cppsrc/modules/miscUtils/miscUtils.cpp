#include "miscUtils.h"
#include <unistd.h>
#include <sys/stat.h>
#include <cstdio>
#include <GLES3/gl32.h>
#include <EGL/egl.h>
#include <string>
#include <sstream>
#include <android/asset_manager_jni.h>
#include <iomanip>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace MiscUtils {
    void DumpNativeImage(NativeImage *pSrcImg, const char *pPath, const char *pFileName, int cnt) {
        if (pSrcImg == nullptr || pPath == nullptr || pFileName == nullptr) return;

        if (access(pPath, 0) == -1) {
            mkdir(pPath, 0666);
        }

        char imgPath[256] = {0};
        const char *pExt = nullptr;
        switch (pSrcImg->format) {
            case IMAGE_FORMAT_I420:
                pExt = IMAGE_FORMAT_I420_EXT;
                break;
            case IMAGE_FORMAT_NV12:
                pExt = IMAGE_FORMAT_NV12_EXT;
                break;
            case IMAGE_FORMAT_NV21:
                pExt = IMAGE_FORMAT_NV21_EXT;
                break;
            case IMAGE_FORMAT_RGBA:
                pExt = IMAGE_FORMAT_RGBA_EXT;
                break;
            case IMAGE_FORMAT_RGB:
                pExt = IMAGE_FORMAT_RGB_EXT;
                break;
            case IMAGE_FORMAT_I420A:
                pExt = IMAGE_FORMAT_I420A_EXT;
                break;
            default:
                pExt = "Default";
                break;
        }

        sprintf(imgPath, "%s/IMG_%dx%d_%s_%d.%s", pPath, pSrcImg->width, pSrcImg->height, pFileName, cnt,
                pExt);

        FILE *fp = fopen(imgPath, "wb");

        ALOGD("DumpNativeImage fp=%p, file=%s", fp, imgPath);

        if (fp) {
            switch (pSrcImg->format) {
                case IMAGE_FORMAT_I420: {
                    fwrite(pSrcImg->ppPlane[0],
                           static_cast<size_t>(pSrcImg->width * pSrcImg->height), 1, fp);
                    fwrite(pSrcImg->ppPlane[1],
                           static_cast<size_t>((pSrcImg->width >> 1) * (pSrcImg->height >> 1)), 1,
                           fp);
                    fwrite(pSrcImg->ppPlane[2],
                           static_cast<size_t>((pSrcImg->width >> 1) * (pSrcImg->height >> 1)), 1,
                           fp);
                    break;
                }
                case IMAGE_FORMAT_I420A: {
                    fwrite(pSrcImg->ppPlane[0],
                           static_cast<size_t>(pSrcImg->width * pSrcImg->height), 1, fp);
                    fwrite(pSrcImg->ppPlane[1],
                           static_cast<size_t>((pSrcImg->width >> 1) * (pSrcImg->height >> 1)), 1,
                           fp);
                    fwrite(pSrcImg->ppPlane[2],
                           static_cast<size_t>((pSrcImg->width >> 1) * (pSrcImg->height >> 1)), 1,
                           fp);
                    fwrite(pSrcImg->ppPlane[3],
                           static_cast<size_t>(pSrcImg->width * pSrcImg->height), 1,
                           fp);
                    break;
                }
                case IMAGE_FORMAT_NV21:
                case IMAGE_FORMAT_NV12: {
                    fwrite(pSrcImg->ppPlane[0],
                           static_cast<size_t>(pSrcImg->width * pSrcImg->height), 1, fp);
                    fwrite(pSrcImg->ppPlane[1],
                           static_cast<size_t>(pSrcImg->width * (pSrcImg->height >> 1)), 1, fp);
                    break;
                }
                case IMAGE_FORMAT_RGBA: {
                    fwrite(pSrcImg->ppPlane[0],
                           static_cast<size_t>(pSrcImg->width * pSrcImg->height * 4), 1, fp);
                    break;
                }
                default: {
                    ALOGD("DumpNativeImage default");
                    break;
                }
            }

            fclose(fp);
            fp = NULL;
        }
    }

    void readPixels (GLuint texID, void* bufferPtr, const int width, const int height)
    {
        GLuint _fbo = 0;
        glGenFramebuffers(1, &_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texID, 0);
        GLenum res = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (res != GL_FRAMEBUFFER_COMPLETE) {
            ALOGE("FBO not complete, error code: %d", res);
            abort();
        }
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, bufferPtr);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &_fbo);

    }

    void dumpPNG(int width, int height, const void *pData, int flipY, int frameCnt, const char* name) {
        std::stringstream ss;
        ss << std::setw(4) << std::setfill('0') << frameCnt;
        std::string formattedFrameCnt = ss.str();
        std::string fileName =
                std::string("/data/local/tmp/") + std::string(name) + formattedFrameCnt + std::string(".png");
        stbi_flip_vertically_on_write(flipY);
        stbi_write_png(fileName.c_str(), width, height, 4, pData, width * 4);
        ALOGM("dump file name: %s", fileName.data());
    }

    void dumpNamePNG(int width, int height, int comp, const void *pData, int flipY, const char* name) {
        std::string fileName = std::string("/data/local/tmp/") + std::string(name) + std::string(".png");
        stbi_flip_vertically_on_write(flipY);
        stbi_write_png(fileName.c_str(), width, height, comp, pData, width * comp);
    }

    void dumpFBO0Color(int width, int height, int flipY, int frameCnt, const char* name) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        uint8_t* pData = (uint8_t*)malloc(4 * width * height);
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pData);
        dumpPNG(width, height, pData, flipY, frameCnt, name);
        free(pData);
    }

    void dumpCurrentFBOColor(int width, int height, int flipY, int frameCnt, const char* name) {
        uint8_t* pData = (uint8_t*)malloc(4 * width * height);
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pData);
        CHECK_GL_ERROR
        dumpPNG(width, height, pData, flipY, frameCnt, name);
        free(pData);
    }

    void dumpTex(unsigned int  tId, int width, int height, int flipY, int frameCnt, const char* name) {
        GLuint _fbo = 0;
        glGenFramebuffers(1, &_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tId, 0);
        GLenum res = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (res != GL_FRAMEBUFFER_COMPLETE) {
            ALOGE("FBO not complete, error code: %d", res);
            abort();
        }
        uint8_t* pData = (uint8_t*)malloc(4 * width * height);
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pData);
        dumpPNG(width, height, pData, flipY, frameCnt, name);
        free(pData);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &_fbo);
    }

    unsigned int loadTextureByBuffer(const unsigned char *dataBuffer, int bufferLen, int* pTexWidth, int* pTexHeight) {
        GLuint loadTexId = 0;
        int texChannels = 0;
        unsigned char *data = NULL;
        stbi_set_flip_vertically_on_load(true);
        data = stbi_load_from_memory(dataBuffer, bufferLen, pTexWidth, pTexHeight, &texChannels, 0);
        ALOGD("hehe, channel: %d, %d, %d", *pTexWidth, *pTexHeight, texChannels);
        GLenum internalFormat = GL_RGBA;
        GLenum format = GL_RGBA;
        switch (texChannels) {
            case 1:
                internalFormat = GL_R8;
                format = GL_RED;
                break;
            case 2:
                internalFormat = GL_RG8;
                format = GL_RG;
                break;
            case 3:
                internalFormat = GL_RGB;
                format = GL_RGB;
                break;
            case 4:
                internalFormat = GL_RGBA;
                format = GL_RGBA;
                break;
            default:
                ALOGE("load texture channel num wrong!!");
                abort();
                break;
        }

        if (data != NULL) {
            glGenTextures(1, &loadTexId);
            glBindTexture(GL_TEXTURE_2D, loadTexId);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, *pTexWidth, *pTexHeight, 0, format, GL_UNSIGNED_BYTE,
                         data);
            glGenerateMipmap(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);
            glUtils::checkGlError(__FILE__, __LINE__);
            stbi_image_free(data);
        } else {
            ALOGE("loadTextureByBuffer error %s", stbi_failure_reason());
        }

        return loadTexId;
    }

    int grtCheckError(int error, const char* errorMsg) {
        if (error != 0) {
            ALOGE("%s, error: %d", errorMsg, error);
            return -1;
        }
        return 0;
    }

    void* justStbImageLoadFromMemory(const unsigned char *dataBuffer, int bufferLen, int* pTexWidth, int* pTexHeight, int* channels)
    {
        return stbi_load_from_memory(
                dataBuffer, bufferLen, pTexWidth,pTexHeight, channels, 4);
    }

    void justStbFreeDataBuffer(void* imageBuffer)
    {
        stbi_image_free(imageBuffer);
    }

    void extractRawAssetData(void*nativeAsset, const char* fileName, std::vector<float>& floatData)
    {
        AAsset *assetFile = AAssetManager_open((AAssetManager *)nativeAsset, fileName, AASSET_MODE_BUFFER);
        size_t bufferLen = AAsset_getLength(assetFile);
        const char * dataBuff = (const char *) AAsset_getBuffer(assetFile);
        char tempWord[50] = {0};
        memset(tempWord, 0, 50);
        int wIdx = 0;
        for (size_t i = 0; i < bufferLen; ++i) {
            if ((dataBuff[i] == ',') || (dataBuff[i] == '\n')) {
                floatData.push_back(atof(tempWord));
                memset(tempWord, 0, 50);
                wIdx = 0;
            } else {
                tempWord[wIdx] = dataBuff[i];
                ++wIdx;
            }
        }
        AAsset_close(assetFile);
    }

    unsigned int loadTextureArrayByBuffers(const std::vector<float>* floatDataArray, int width, int height, int depth,
                                           GLenum internalFormat, GLenum format, GLenum dataType)
    {
        GLuint loadTexId = 0;
        assert(floatDataArray != NULL);
        glGenTextures(1, &loadTexId);
        glBindTexture(GL_TEXTURE_2D_ARRAY, loadTexId);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);CHECK_GL_ERROR
        glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, internalFormat, width, height, depth);CHECK_GL_ERROR
        for (int i = 0; i < depth; ++i) {
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, format, dataType, floatDataArray[i].data());
            CHECK_GL_ERROR
        }
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        CHECK_GL_ERROR
        return loadTexId;
    }

    void getShaderStringFromAsset(AAssetManager* assetMgr, const char* fileName, std::string& outString)
    {
        // open shader file
        AAsset* shaderAssetFile = AAssetManager_open(assetMgr, fileName, AASSET_MODE_BUFFER);
        YY_DEMO_ASSERT(shaderAssetFile != nullptr);
        size_t shaderBufferLen = AAsset_getLength(shaderAssetFile);
        YY_DEMO_ASSERT(shaderBufferLen != 0);
        //  additVonal terminating null-character ('\0') at the end automatically when c_str() return
        outString = std::string( static_cast<const char*>(AAsset_getBuffer(shaderAssetFile)), shaderBufferLen);
        AAsset_close(shaderAssetFile);
    }

    long getCurrentTimeInSeconds() {
        // Get the current time point
        auto currentTime = std::chrono::system_clock::now();

        // Calculate the duration since the epoch (usually January 1, 1970)
        auto duration = currentTime.time_since_epoch();

        // Convert the duration to seconds
        long seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();

        return seconds;
    }

    void getV4RDataFromFile(AAssetManager* assetMgr, const char* fileName, std::vector<float>& outVec)
    {
        AAsset* shaderAssetFile = AAssetManager_open(assetMgr, fileName, AASSET_MODE_BUFFER);
        size_t shaderBufferLen = AAsset_getLength(shaderAssetFile);
        //  additVonal terminating null-character ('\0') at the end automatically when c_str() return
        std::string outString = std::string( static_cast<const char*>(AAsset_getBuffer(shaderAssetFile)), shaderBufferLen);
        AAsset_close(shaderAssetFile);
        std::istringstream stream(outString);
        std::string line;
        while (std::getline(stream, line)) {
            // 处理每一行
            outVec.push_back(std::stof(line));
        }
    }

    void getGazeDataFromFile(AAssetManager* assetMgr, const char* fileName, std::vector<float>& outVec)
    {
        AAsset* shaderAssetFile = AAssetManager_open(assetMgr, fileName, AASSET_MODE_BUFFER);
        size_t shaderBufferLen = AAsset_getLength(shaderAssetFile);
        //  additVonal terminating null-character ('\0') at the end automatically when c_str() return
        std::string outString = std::string( static_cast<const char*>(AAsset_getBuffer(shaderAssetFile)), shaderBufferLen);
        AAsset_close(shaderAssetFile);
        std::istringstream stream(outString);
        std::string line;
        while (std::getline(stream, line)) {
            // 处理每一行
            outVec.push_back(std::stof(line));
        }
    }

    void YYDemoAssert(bool cond, const char* fileName, int lineNum) {
        if (!cond) {
            ALOGE("YY Assert in file %s, at line %d\n", fileName, lineNum);
            abort();
        }
    }

}

namespace glUtils {
    unsigned int loadTextureFromAssetFile(void* nativeAsset, const char* filename, int* width, int* height)
    {
        unsigned int retTex = 0;
        AAsset *assetFile = AAssetManager_open((AAssetManager *)nativeAsset, filename, AASSET_MODE_BUFFER);
        size_t bufferLen = AAsset_getLength(assetFile);

        const unsigned char *dataBuff = (const unsigned char *) AAsset_getBuffer(assetFile);
        retTex = MiscUtils::loadTextureByBuffer(dataBuff, bufferLen, width, height);
        AAsset_close(assetFile);
        return retTex;
    }

    unsigned int loadHDREnvTextureFromAssetFile(void* nativeAsset, const char* filename, int& width, int& height)
    {
        unsigned int texID = 0;
        AAsset *assetFile = AAssetManager_open((AAssetManager *)nativeAsset, filename, AASSET_MODE_BUFFER);
        YY_DEMO_ASSERT(assetFile != nullptr);
        size_t bufferLen = AAsset_getLength(assetFile);

        const unsigned char *dataBuff = (const unsigned char *) AAsset_getBuffer(assetFile);

        stbi_set_flip_vertically_on_load(true);
        int _channel = 0;
        float* floatDataPtr = stbi_loadf_from_memory(dataBuff, bufferLen, &width, &height, &_channel, 0);
        if (floatDataPtr != nullptr) {
            glGenTextures(1, &texID);
            glBindTexture(GL_TEXTURE_2D, texID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, floatDataPtr);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glBindTexture(GL_TEXTURE_2D, 0);
            CHECK_GL_ERROR
            stbi_image_free(floatDataPtr);
        } else {
            ALOGE("load hdr image error");
        }
        return texID;
    }

    int64_t getTimeNsec() {
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        return (int64_t) now.tv_sec * 1000000000LL + now.tv_nsec;
    }

    double showTimeDiff(int64_t t1, int64_t t2, double drawAvgTime, int frameCnt, const char* msg)
    {
        double drawTime = (t2 - t1) / 1000000.0;
        drawAvgTime += (drawTime - drawAvgTime) / frameCnt;
        ALOGM("yyal %s avg draw time: %f ms, frame cnt: %d", msg, drawAvgTime, frameCnt);
        return drawAvgTime;
    }
    void showTimeDiffAbs(int64_t t1, int64_t t2, const char* msg)
    {
        double drawTime = (t2 - t1) / 1000000.0;
        ALOGM("yyal %s abs draw time: %f ms", msg, drawTime);
    }


    void checkEGlError(const char* fileName, int lineNum)
    {
        GLint err = eglGetError();
        if (err != EGL_SUCCESS) {
            ALOGE("EGL error: 0x%08x in file %s, at line %d\n", err, fileName, lineNum);
            abort();
        }
    }
    void checkGlError(const char* fileName, int lineNum)
    {
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            ALOGE("GL error: 0x%08x in file %s, at line %d\n", err, fileName, lineNum);
            abort();
        }
    }


    void grAbort(const char* fileName, int lineNum)
    {
        ALOGE("Abort happen in %s, at line %d", fileName, lineNum);
        abort();
    }

    GLuint hardwareBufferToTexture(AHardwareBuffer* buffer, GLint texType, GLint filter, EGLImage* retEGLImage, bool useSRGB)
    {
        p_eglGetNativeClientBufferANDROID = (PFNEGLGETNATIVECLIENTBUFFERANDROIDPROC)eglGetProcAddress("eglGetNativeClientBufferANDROID");
        EGLClientBuffer clientBuffer =  p_eglGetNativeClientBufferANDROID(buffer);
        checkEGlError(__FILE__, __LINE__);

        p_eglCreateImageKHR = (PFNEGLCREATEIMAGEKHR) eglGetProcAddress("eglCreateImageKHR");
        EGLint attrib_lists[3] = {EGL_IMAGE_PRESERVED_KHR, EGL_TRUE,
                                  EGL_NONE };
        EGLint attrib_lists_srgb[5] = {EGL_IMAGE_PRESERVED_KHR, EGL_TRUE,
                               EGL_GL_COLORSPACE, EGL_GL_COLORSPACE_SRGB,
                               EGL_NONE };

        EGLImageKHR eglImage = p_eglCreateImageKHR(eglGetCurrentDisplay(),
                                                   EGL_NO_CONTEXT,
                                                   EGL_NATIVE_BUFFER_ANDROID,
                                                   clientBuffer,
                                                   useSRGB ? attrib_lists_srgb : attrib_lists);
        checkEGlError(__FILE__, __LINE__);

        GLuint texID = 0;
        glGenTextures(1, &texID);
        checkGlError(__FILE__, __LINE__);
        glBindTexture(texType, texID);

        p_glEGLImageTargetTexture2DOES = (PFNEGLIMAGETARGETTEXTURE2DOES) eglGetProcAddress("glEGLImageTargetTexture2DOES");
        p_glEGLImageTargetTexture2DOES(texType, eglImage);
        glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, filter);
        glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, filter);

        glBindTexture(texType, 0);
        checkGlError(__FILE__, __LINE__);

        *retEGLImage = eglImage;
        return texID;
    }

    void deleteEGLResources(GLuint texID, EGLImage image)
    {
        glDeleteTextures(1, &texID);
        eglDestroyImage(eglGetCurrentDisplay(), image);
        glUtils::checkGlError(__FILE__, __LINE__);
        glUtils::checkEGlError(__FILE__, __LINE__);
    }

    GLuint genTexture(int texWidth, int texHeight)
    {
        GLuint texID = 0;
        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_2D, texID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, texWidth, texHeight);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUtils::checkGlError(__FILE__, __LINE__);
        return texID;
    }

    const char* getShaderString(GLenum shaderType)
    {
        const char* res = "shader";
        switch (shaderType) {
            case GL_VERTEX_SHADER:
                res = "vertex";
                break;
            case GL_FRAGMENT_SHADER:
                res = "fragment";
                break;
            case GL_COMPUTE_SHADER:
                res = "compute";
                break;
        }
        return res;
    }

    GLuint createShader(GLenum shaderType, const char *src) {
        GLuint shader = glCreateShader(shaderType);
        if (!shader) {
            CHECK_GL_ERROR
            return 0;
        }
        glShaderSource(shader, 1, &src, NULL);
        GLint compiled = GL_FALSE;
        glCompileShader(shader);
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLogLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);
            if (infoLogLen > 0) {
                GLchar *infoLog = (GLchar *) malloc(infoLogLen);
                if (infoLog) {
                    glGetShaderInfoLog(shader, infoLogLen, NULL, infoLog);
                    ALOGE("yyaolong Could not compile %s shader:\n%s\n",
                          getShaderString(shaderType),
                          infoLog);
                    free(infoLog);
                }
            }
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    GLuint createProgram(const char *vtxSrc, const char *fragSrc) {
        GLuint vtxShader = 0;
        GLuint fragShader = 0;
        GLuint program = 0;
        GLint linked = GL_FALSE;
        vtxShader = createShader(GL_VERTEX_SHADER, vtxSrc);
        YY_DEMO_ASSERT(vtxShader != 0)
        fragShader = createShader(GL_FRAGMENT_SHADER, fragSrc);
        YY_DEMO_ASSERT(fragShader != 0)
        program = glCreateProgram();
        YY_DEMO_ASSERT(program != 0)
        glAttachShader(program, vtxShader);
        glAttachShader(program, fragShader);
        glLinkProgram(program);
        glGetProgramiv(program, GL_LINK_STATUS, &linked);
        if (!linked) {
            ALOGE("Could not link program");
            GLint infoLogLen = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLen);
            if (infoLogLen) {
                GLchar *infoLog = (GLchar *) malloc(infoLogLen);
                if (infoLog) {
                    glGetProgramInfoLog(program, infoLogLen, NULL, infoLog);
                    ALOGE("yyaolong Could not link program:\n%s\n", infoLog);
                    free(infoLog);
                }
            }
            abort();
            glDeleteProgram(program);
            program = 0;
        }
        glDeleteShader(vtxShader);
        glDeleteShader(fragShader);
        return program;
    }

    GLuint createProgramFromAssetName(void* assetMgr, const char *vsFileName, const char *fsFileName)
    {
        GLuint program = 0;
        std::string strVs;
        std::string strFs;
        ALOGD("compile vs: %s and fs: %s", vsFileName, fsFileName);
        MiscUtils::getShaderStringFromAsset(static_cast<AAssetManager*>(assetMgr), vsFileName, strVs);
        MiscUtils::getShaderStringFromAsset(static_cast<AAssetManager*>(assetMgr), fsFileName, strFs);
        program = glUtils::createProgram(strVs.c_str(), strFs.c_str());
        return program;
    }

    GLuint createTexture(int width,
                         int height,
                         GLenum textureTarget,
                         GLenum internalFormat,
                         GLenum format,
                         int depth,
                         int mipmapLevels,
                         GLenum type,
                         void* data,
                         GLint minFilter,
                         GLint maxFilter ,
                         GLint wrapWay,
                         bool immutable,
                         bool anisotropy)
    {
        GLuint retTexID = 0;
        glGenTextures(1, &retTexID);
        glBindTexture(textureTarget, retTexID);
        glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, minFilter);
        glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, maxFilter);
        if (anisotropy) {
            GLfloat largestAnisotropic = 0.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largestAnisotropic);
            glTexParameterf(textureTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, largestAnisotropic);
        }
        glTexParameteri(textureTarget,GL_TEXTURE_WRAP_S,wrapWay);
        glTexParameteri(textureTarget,GL_TEXTURE_WRAP_T,wrapWay);
        glTexParameteri(textureTarget,GL_TEXTURE_WRAP_R,wrapWay);
        if (immutable) {
            if ((textureTarget == GL_TEXTURE_2D_ARRAY) ||
                (textureTarget == GL_TEXTURE_3D)) {
                glTexStorage3D(textureTarget,
                               mipmapLevels,
                               internalFormat, width, height, depth);
            } else if ((textureTarget == GL_TEXTURE_2D) ||
                       (textureTarget == GL_TEXTURE_CUBE_MAP)) {
                glTexStorage2D(textureTarget,
                               mipmapLevels,
                               internalFormat, width, height);
            }  else {
                YYDEMO_ABORT
            }
        } else {
            if ((textureTarget == GL_TEXTURE_2D_ARRAY) ||
                (textureTarget == GL_TEXTURE_3D)) {
                glTexImage3D(textureTarget,
                             0,
                             static_cast<GLint>(internalFormat), width, height, depth,
                             0, format, type, data);
            } else if (textureTarget == GL_TEXTURE_2D) {
                glTexImage2D(textureTarget,
                             0,
                             static_cast<GLint>(internalFormat), width, height,
                             0, format, type, data);
            } else if (textureTarget == GL_TEXTURE_CUBE_MAP) {
                for (unsigned int i = 0; i < 6; ++i)
                {
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                 0,
                                 static_cast<GLint>(internalFormat),
                                 width, height,0, format, type, data);
                }
            } else {
                YYDEMO_ABORT
            }
        }
        glBindTexture(textureTarget, 0);
        CHECK_GL_ERROR
        return retTexID;
    }

    GLuint createDepthStencilRbo (int width, int height, GLenum internalFormat)
    {
        GLuint rbo = 0;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        return rbo;
    }

    GLuint genAnisotropyTexture(int texWidth, int texHeight)
    {
        GLuint texID = 0;
        GLfloat largestAnisotropic = 0.0f;
        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_2D, texID);
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largestAnisotropic);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largestAnisotropic);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUtils::checkGlError(__FILE__, __LINE__);
        return texID;
    }

    void enableColorTexFoveation(GLenum texType, GLuint texID, bool enableSubSampleLayout, float gazeH, float gazeV) {
        glBindTexture(texType, texID);
        GLuint requestedFeatures = GL_FOVEATION_ENABLE_BIT_QCOM | GL_FOVEATION_SCALED_BIN_METHOD_BIT_QCOM;
        if (enableSubSampleLayout) {
            requestedFeatures = GL_FOVEATION_ENABLE_BIT_QCOM | GL_FOVEATION_SUBSAMPLED_LAYOUT_METHOD_BIT_QCOM;
        }
        glTexParameteri(texType, GL_TEXTURE_FOVEATED_FEATURE_BITS_QCOM, requestedFeatures);
        p_glTextureFoveationParametersQCOM = (PFNGLTextureFoveationParametersQCOM)(eglGetProcAddress("glTextureFoveationParametersQCOM"));
        p_glTextureFoveationParametersQCOM(texID, 0, 0,
                                           gazeH, gazeV,
                                           2.6458,
                                           2.6458,
                                           3.0);CHECK_GL_ERROR
        p_glTextureFoveationParametersQCOM(texID, 1, 0,
                                           gazeH, gazeV,
                                           2.6458,
                                           2.6458,
                                           3.0);CHECK_GL_ERROR
        glBindTexture(texType, 0);
    }

    void initAllFuncAddress()
    {
        g_glFramebufferTexture2DMultisampleEXT = (PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEIMG)eglGetProcAddress ("glFramebufferTexture2DMultisampleEXT");
        g_glRenderbufferStorageMultisampleEXT = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEIMG) eglGetProcAddress("glRenderbufferStorageMultisampleEXT");
        p_glFramebufferTextureMultiviewOVR = (PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVR)eglGetProcAddress ("glFramebufferTextureMultiviewOVR");
        p_glFramebufferTextureMultisampleMultiviewOVR = (PFNGLFRAMEBUFFERTEXTUREMULTISAMPLEMULTIVIEWOVR)eglGetProcAddress ("glFramebufferTextureMultisampleMultiviewOVR");
    }

    // utility uniform functions
    // ------------------------------------------------------------------------
    void setUniformBool(GLuint program, const char* name, bool value)
    {
        glUniform1i(glGetUniformLocation(program, name), (int)value);
    }
    // ------------------------------------------------------------------------
    void setUniformInt(GLuint program, const char* name, int value)
    {
        glUniform1i(glGetUniformLocation(program, name), value);
    }
    // ------------------------------------------------------------------------
    void setUniformFloat(GLuint program, const char* name, float value)
    {
        glUniform1f(glGetUniformLocation(program, name), value);
    }
    // ------------------------------------------------------------------------
    void setUniformVec2(GLuint program, const char* name, const float* ptr)
    {
        glUniform2fv(glGetUniformLocation(program, name), 1, ptr);
    }
    void setUniformVec2(GLuint program, const char* name, float x, float y)
    {
        glUniform2f(glGetUniformLocation(program, name), x, y);
    }
    // ------------------------------------------------------------------------
    void setUniformVec3(GLuint program, const char* name, const float* ptr)
    {
        glUniform3fv(glGetUniformLocation(program, name), 1, ptr);
    }
    void setUniformVec3(GLuint program, const char* name, float x, float y, float z)
    {
        glUniform3f(glGetUniformLocation(program, name), x, y, z);
    }
    // ------------------------------------------------------------------------
    void setUniformVec4(GLuint program, const char* name, const float* ptr)
    {
        glUniform4fv(glGetUniformLocation(program, name), 1, ptr);
    }
    void setUniformVec4(GLuint program, const char* name, float x, float y, float z, float w)
    {
        glUniform4f(glGetUniformLocation(program, name), x, y, z, w);
    }
    // ------------------------------------------------------------------------
    void setUniformMat2(GLuint program, const char* name, const float* ptr)
    {
        glUniformMatrix2fv(glGetUniformLocation(program, name), 1, GL_FALSE, ptr);
    }
    // ------------------------------------------------------------------------
    void setUniformMat3(GLuint program, const char* name, const float* ptr)
    {
        glUniformMatrix3fv(glGetUniformLocation(program, name), 1, GL_FALSE, ptr);
    }
    // ------------------------------------------------------------------------
    void setUniformMat4(GLuint program, const char* name, const float* ptr)
    {
        glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE, ptr);
    }

    void generateSphereVboAndEboData(std::vector<float>& vboData, std::vector<unsigned int>& eboData)
    {
        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.14159265359f;
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
        {
            for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                // position
                vboData.push_back(xPos);
                vboData.push_back(yPos);
                vboData.push_back(zPos);
                // normal
                vboData.push_back(xPos);
                vboData.push_back(yPos);
                vboData.push_back(zPos);
                // UV
                vboData.push_back(xSegment);
                vboData.push_back(ySegment);

            }
        }

        bool oddRow = false;
        for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
        {
            if (!oddRow) // even rows: y == 0, y == 2; and so on
            {
                for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
                {
                    eboData.push_back(y * (X_SEGMENTS + 1) + x);
                    eboData.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                }
            }
            else
            {
                for (int x = X_SEGMENTS; x >= 0; --x)
                {
                    eboData.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    eboData.push_back(y * (X_SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;
        }
    }
}