//
// Created by yuyao on 2024/2/18.
//

#pragma once

#include "YYPlatform.h"

#include <cstdlib>
#include <ctime>
#include <chrono>
#include <cstring>
#include <cassert>

#include <vector>
#include <cmath>
#include <algorithm>
#include <string>
#include <memory>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

#ifndef LOG_TAG
#define LOG_TAG "YYDemo"
#endif

#define STR(s) #s
#define STRV(s) STR(s)

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WINAPI_FAMILY)
    #define YYGL_INTERFACE_EXPORT __declspec(dllexport)
    #define YYGL_INTERFACE_IMPORT __declspec(dllimport)
    #define UNITY_INTERFACE_API __stdcall
#elif defined(__GNUC__)
    #define YYGL_INTERFACE_EXPORT __attribute__((visibility("default")))
    #define YYGL_INTERFACE_IMPORT
    #define UNITY_INTERFACE_API
#else
    #define YYGL_INTERFACE_EXPORT
    #define YYGL_INTERFACE_IMPORT
    #define UNITY_INTERFACE_API
#endif

#ifdef CPPUTILS_EXPORTS
    #define YYGL_INTERFACE_API YYGL_INTERFACE_EXPORT
#else
    #define YYGL_INTERFACE_API YYGL_INTERFACE_IMPORT
#endif


#ifdef __ANDROID__

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
#define CHECK_EGL_ERROR cppUtils::checkEGLError(__FILE__, __LINE__);
#endif

#define CHECK_GL_ERROR cppUtils::checkGlError(__FILE__, __LINE__);

#define YYDEMO_ABORT cppUtils::grAbort(__FILE__, __LINE__);
#define YY_DEMO_ASSERT(cond) cppUtils::YYDemoAssert(cond, __FILE__, __LINE__);

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

namespace cppUtils {

    typedef struct _foveationParamters {
        double fovArea;
        double gainX;
        double gainY;
        double focalX;
        double focalY;
    } FoveationParameters;

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

#ifdef __ANDROID__
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

    void getShaderStringFromAsset(AAssetManager* assetMgr, const char* fileName, std::string& outString);
    void getV4RDataFromFile(AAssetManager* assetMgr, const char* fileName, std::vector<float>& outVec);
    void getGazeDataFromFile(AAssetManager* assetMgr, const char* fileName, std::vector<float>& outVec);

    GLuint hardwareBufferToTexture(AHardwareBuffer* buffer, GLint texType, GLint filter, EGLImage* retEGLImage, bool useSRGB = false);
    void deleteEGLResources(GLuint texID, EGLImage image);

    void checkEGLError(const char* fileName, int lineNum);
    void DumpNativeImage(NativeImage *pSrcImg, const char *pPath, const char *pFileName, int cnt) ;
    void extractRawAssetData(void*nativeAsset, const char* fileName, std::vector<float>& floatData);
    unsigned int loadHDREnvTextureFromAssetFile(void* nativeAsset, const char* filename, int& width, int& height);
#endif
#ifdef _WIN32
    bool loadGLFuncAddressByGlad();
#endif
    unsigned int loadTextureFromFile(void* nativeAsset, const char* filename, int* width, int* height, bool flipUV = true);
    void dumpFBO0Color(int width, int height, int flipY, int frameCnt, const char* name);
    void dumpCurrentFBOColor(int width, int height, int flipY, int frameCnt, const char* name);
    void dumpTex(unsigned int tId, int width, int height, int flipY, int frameCnt, const char* name);
    void dumpPNG(int width, int height, const void *pData, int flipY, int frameCnt, const char* name);
    void dumpNamePNG(int width, int height, int comp, const void *pData, int flipY, const char* name);
    unsigned int loadTextureByBuffer(const unsigned char *dataBuffer, int bufferLen, int* pTexWidth, int* pTexHeight, bool flipUV = true);
    void readPixels (GLuint texID, void* bufferPtr, const int width, const int height);
    int grtCheckError(int error, const char* errorMsg);
    void* justStbImageLoadFromMemory(const unsigned char *dataBuffer, int bufferLen, int* pTexWidth, int* pTexHeight, int* channels);
    void justStbFreeDataBuffer(void* imageBuffer);
    unsigned int loadTextureArrayByBuffers(const std::vector<float>* floatDataArray, int width, int height, int depth,
                                           GLenum internalFormat, GLenum format, GLenum dataType);
    long getCurrentTimeInSeconds();
    void YYDemoAssert(bool cond, const char* fileName, int lineNum);

    int64_t getTimeNsec();
    double showTimeDiff(int64_t t1, int64_t t2, double drawAvgTime, int frameCnt, const char* msg);
    void showTimeDiffAbs(int64_t t1, int64_t t2, const char* msg);
    void checkGlError(const char* fileName, int lineNum);
    void grAbort(const char* fileName, int lineNum);
    GLuint genTexture(int texWidth, int texHeight);
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

namespace YYGLModelData {
    extern YYGL_INTERFACE_API float DEPTH_TEST_VERTEX_DATA[30];

    extern YYGL_INTERFACE_API float QUAD_TEST_VERTEX_DATA[30];


    extern YYGL_INTERFACE_API float RECTANGLE_POS2_TEXCOOR2[24];

    extern YYGL_INTERFACE_API float CUBE_POS3_NORMAL3_TEXCOOR2[288];

#ifdef _WIN32
    // model data from assimp
    #define MAX_BONE_INFLUENCE 4
    struct AssimpVertex {
        // position
        glm::vec3 Position;
        // normal
        glm::vec3 Normal;
        // texCoords
        glm::vec2 TexCoords;
        // tangent
        glm::vec3 Tangent;
        // bitangent
        glm::vec3 Bitangent;
        //bone indexes which will influence this vertex
        int m_BoneIDs[MAX_BONE_INFLUENCE];
        //weights from each bone
        float m_Weights[MAX_BONE_INFLUENCE];
    };

    struct AssimpTexture {
        unsigned int id;
        std::string type;
        std::string path;
    };

    class AssimpMesh {
    public:
        // mesh Data
        std::vector<AssimpVertex>  vertices;
        std::vector<unsigned int>  indices;
        std::vector<AssimpTexture> textures;
        unsigned int VAO;

        // constructor
        AssimpMesh(std::vector<AssimpVertex> vertices, std::vector<unsigned int> indices, std::vector<AssimpTexture> textures)
        {
            this->vertices = vertices;
            this->indices = indices;
            this->textures = textures;

            // now that we have all the required data, set the vertex buffers and its attribute pointers.
            setupMesh();
        }

        // render the mesh
        void Draw(unsigned int program);

    private:
        // render data 
        unsigned int VBO, EBO;

        // initializes all the buffer objects/arrays
        void setupMesh();
    };

    class AssimpModel
    {
    public:
        // model data
        // stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
        std::vector<AssimpTexture> textures_loaded;
        std::vector<AssimpMesh>    meshes;
        std::string directory;
        bool mFlipV;

        // constructor, expects a filepath to a 3D model.
        AssimpModel(std::string const &path, bool flipV = false) : mFlipV(flipV)
        {
            loadModel(path);
        }

        // draws the model, and thus all its meshes
        void Draw(unsigned int program)
        {
            for(unsigned int i = 0; i < meshes.size(); i++)
                meshes[i].Draw(program);
        }
        
    private:
        // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
        void loadModel(std::string const &path);
        // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
        void processNode(const void* node, const void* scene);
        AssimpMesh processMesh(const void* mesh, const void* scene);
        // checks all material textures of a given type and loads the textures if they're not loaded yet.
        // the required info is returned as a Texture struct.
        std::vector<AssimpTexture> loadMaterialTextures(const void* mat, int type, std::string typeName);
    };
#endif
}