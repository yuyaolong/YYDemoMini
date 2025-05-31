//
// Copyright (c) 2022-present, YYAL.  All rights reserved.
// Created by Yaolong Yu on 2022/8/11.
//

#include "pbrRender.h"
#include "miscUtils.h"
#include "YYGLModule.hpp"

namespace {
    const int SH9UBOFloatSize = 9 * 4;
    struct PbrRenderModuleData : YYGLModuleData {
        glm::vec3 mLightPositions[4] = {
                glm::vec3(-9.0f,  9.0f, 9.0f),
                glm::vec3( 9.0f,  9.0f, 9.0f),
                glm::vec3(-9.0f, -9.0f, 9.0f),
                glm::vec3( 9.0f, -9.0f, 9.0f)
        };
        glm::vec3 mLightColors[4] = {
                glm::vec3(300.0f, 300.0f, 300.0f),
                glm::vec3(300.0f, 300.0f, 300.0f),
                glm::vec3(300.0f, 300.0f, 300.0f),
                glm::vec3(300.0f, 300.0f, 300.0f)
        };
        glm::vec3 mViewPosition;
        float mSH9UboData[SH9UBOFloatSize] = {0.0f};
    };
    class PbrRenderModule : YYGLModule {
    public:
        explicit PbrRenderModule(const YYGLModuleData &mModuleData) : YYGLModule(mModuleData) {}
        ~PbrRenderModule() override{
            glDeleteBuffers(1, &mSH9Ubo);
        };

         void grInitModule() override {
             YYGLModule::grInitModule();
             glUseProgram(mProgram);
             for (YYTextureData texData : mModuleData.mSourceTexArray ) {
                 glUtils::setUniformInt(mProgram, texData.texUName, texData.texUniformIdx);
             }

             // set lights uniforms
             const PbrRenderModuleData& pbrRenderModuleData = dynamic_cast<const PbrRenderModuleData&>(mModuleData);
             for (unsigned int i = 0; i < sizeof(pbrRenderModuleData.mLightPositions) / sizeof(pbrRenderModuleData.mLightPositions[0]); ++i)
             {
                 std::string lightPos = "lightPositions[" + std::to_string(i) + "]";
                 glUniform3fv(glGetUniformLocation(mProgram, lightPos.c_str()), 1, &(pbrRenderModuleData.mLightPositions[i][0]));
                 std::string lightColor = "lightColors[" + std::to_string(i) + "]";
                 glUniform3fv(glGetUniformLocation(mProgram, lightColor.c_str()), 1, &(pbrRenderModuleData.mLightColors[i][0]));
                 CHECK_GL_ERROR
             }

             GLuint uniformBlockIndex = glGetUniformBlockIndex(mProgram, "sh9Coef");
             glUniformBlockBinding(mProgram, uniformBlockIndex, mSH9UboBlockBindingIndex);

             glUseProgram(0);

             // generate UBO
             glGenBuffers(1, &mSH9Ubo);
             glBindBuffer(GL_UNIFORM_BUFFER, mSH9Ubo);
             glBufferData(GL_UNIFORM_BUFFER, SH9UBOFloatSize * sizeof(float), nullptr, GL_STATIC_DRAW);
             glBindBufferBase(GL_UNIFORM_BUFFER, mSH9UboBlockBindingIndex, mSH9Ubo);
             glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }

        unsigned int grProcessModule() override {
            const PbrRenderModuleData& pbrRenderModuleData = dynamic_cast<const PbrRenderModuleData&>(mModuleData);
             // update UBO
            glBindBuffer(GL_UNIFORM_BUFFER, mSH9Ubo);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, SH9UBOFloatSize * sizeof(float), pbrRenderModuleData.mSH9UboData);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);

             // update MVP matrix
            glUseProgram(mProgram);
            glUniformMatrix4fv(glGetUniformLocation(mProgram, "model"), 1, GL_FALSE, &(pbrRenderModuleData.mModelMat4[0][0]));
            glUniformMatrix4fv(glGetUniformLocation(mProgram, "view"), 1, GL_FALSE, &(pbrRenderModuleData.mViewMat4[0][0]));
            glUniformMatrix4fv(glGetUniformLocation(mProgram, "projection"), 1, GL_FALSE, &(pbrRenderModuleData.mProjectionMat4[0][0]));
            glm::mat3 normalMat3 = glm::transpose(glm::inverse(glm::mat3(pbrRenderModuleData.mModelMat4)));
            glUniformMatrix3fv(glGetUniformLocation(mProgram, "normalMatrix"), 1, GL_FALSE, &normalMat3[0][0]);
            glUniform3fv(glGetUniformLocation(mProgram, "camPos"), 1, &(pbrRenderModuleData.mViewPosition[0]));
            glUseProgram(0);
            return YYGLModule::grProcessModule();
         }

        PbrRenderModule(const PbrRenderModule&) = delete;
        PbrRenderModule& operator=(const PbrRenderModule&) = delete;

    private:
        const GLuint mSH9UboBlockBindingIndex = 0;
        GLuint mSH9Ubo = 0;
    };

    PbrRenderModuleData* g_moduleDataPtr = nullptr;
    PbrRenderModule* g_modulePtr = nullptr;

}

// interface
void grInitPbrRender(void* assetMgr,
                     const float* modelDataPtr,
                     int modelDataLen,
                     const unsigned int* elementDataPtr,
                     int elementDataLen,
                     int drawType,
                     const int* attributesSizeArray,
                     int attributesSizeArrayLen,
                     int verticesNum,
                     const char* materialFolder,
                     const float* SH9_vec4s)
{
    g_moduleDataPtr = new PbrRenderModuleData();
    YY_DEMO_ASSERT(g_moduleDataPtr != nullptr)
    g_modulePtr = new PbrRenderModule(*g_moduleDataPtr);
    YY_DEMO_ASSERT(g_modulePtr != nullptr)
    g_moduleDataPtr->mAssetMgr = assetMgr;
    g_moduleDataPtr->mVsFileName = "shaders/pbr/pbr.vs";
    g_moduleDataPtr->mFsFileName = "shaders/pbr/pbr.fs";
    g_moduleDataPtr->mModelDataPtr = modelDataPtr;
    g_moduleDataPtr->mModelDataLen = modelDataLen;
    g_moduleDataPtr->mDrawType = drawType;
    if (elementDataPtr != nullptr) {
        g_moduleDataPtr->mEnableElementDraw = true;
        g_moduleDataPtr->mElementDataPtr = elementDataPtr;
        YY_DEMO_ASSERT(elementDataLen != 0);
        g_moduleDataPtr->mElementDataLen = elementDataLen;
    }
    g_moduleDataPtr->mAttributesSizeArray = std::vector<int>(attributesSizeArray, attributesSizeArray + attributesSizeArrayLen);
    g_moduleDataPtr->mEnableDepthTest = true;
    g_moduleDataPtr->mVertexOrIndexNum = verticesNum;
    g_moduleDataPtr->mSourceTexArray.resize(8);

    memcpy(g_moduleDataPtr->mSH9UboData, SH9_vec4s, SH9UBOFloatSize * sizeof(float ));

    int texWidth = 0;
    int texHeight = 0;
    std::string prefix("pbrTextures/");
    std::string fileName = prefix + std::string(materialFolder) + std::string("/albedo.png");
    unsigned int albedoMapTex = glUtils::loadTextureFromAssetFile(assetMgr, fileName.c_str(), &texWidth, &texHeight);
    fileName = prefix + std::string(materialFolder) + std::string("/normal.png");
    unsigned int normalMapTex = glUtils::loadTextureFromAssetFile(assetMgr, fileName.c_str(), &texWidth, &texHeight);
    fileName = prefix + std::string(materialFolder) + std::string("/metallic.png");
    unsigned int metallicMapTex = glUtils::loadTextureFromAssetFile(assetMgr, fileName.c_str(), &texWidth, &texHeight);
    fileName = prefix + std::string(materialFolder) + std::string("/roughness.png");
    unsigned int roughnessMapTex = glUtils::loadTextureFromAssetFile(assetMgr, fileName.c_str(), &texWidth, &texHeight);
    fileName = prefix + std::string(materialFolder) + std::string("/ao.png");
    unsigned int aoMapTex = glUtils::loadTextureFromAssetFile(assetMgr, fileName.c_str(), &texWidth, &texHeight);
    g_moduleDataPtr->mSourceTexArray[0] = {albedoMapTex, GL_TEXTURE_2D, 0, "albedoMap"};
    g_moduleDataPtr->mSourceTexArray[1] = {normalMapTex, GL_TEXTURE_2D, 1, "normalMap"};
    g_moduleDataPtr->mSourceTexArray[2] = {metallicMapTex, GL_TEXTURE_2D, 2, "metallicMap"};
    g_moduleDataPtr->mSourceTexArray[3] = {roughnessMapTex, GL_TEXTURE_2D, 3, "roughnessMap"};
    g_moduleDataPtr->mSourceTexArray[4] = {aoMapTex, GL_TEXTURE_2D, 4, "aoMap"};
    g_moduleDataPtr->mSourceTexArray[5] = {0, GL_TEXTURE_CUBE_MAP, 5, "irradianceMap"};
    g_moduleDataPtr->mSourceTexArray[6] = {0, GL_TEXTURE_CUBE_MAP, 6, "prefilterMap"};
    g_moduleDataPtr->mSourceTexArray[7] = {0, GL_TEXTURE_2D, 7, "brdfLUT"};

    g_modulePtr->grInitModule();

}

unsigned int grProcessPbrRender(unsigned int irradianceMapTexID,
                                unsigned int specularPrefilterMapTexID,
                                unsigned int specularBRDFLutTexID,
                                int targetWidth,
                                int targetHeight,
                                unsigned int targetColorTexID,
                                unsigned int targetDepthTexID,
                                unsigned int targetDepthRbo,
                                void* modelMat4,
                                void* viewMat4,
                                void* projectionMat4,
                                void* viewPositionVec3,
                                bool clearColor)
{
    YY_DEMO_ASSERT(g_moduleDataPtr != nullptr);
    g_moduleDataPtr->mSourceTexArray[5].texID = irradianceMapTexID;
    g_moduleDataPtr->mSourceTexArray[5].texTarget = GL_TEXTURE_CUBE_MAP;
    g_moduleDataPtr->mSourceTexArray[6].texID = specularPrefilterMapTexID;
    g_moduleDataPtr->mSourceTexArray[6].texTarget = GL_TEXTURE_CUBE_MAP;
    g_moduleDataPtr->mSourceTexArray[7].texID = specularBRDFLutTexID;
    g_moduleDataPtr->mSourceTexArray[7].texTarget = GL_TEXTURE_2D;
    g_moduleDataPtr->mTargetWidth = targetWidth;
    g_moduleDataPtr->mTargetHeight = targetHeight;
    g_moduleDataPtr->mColorTargetTexID = targetColorTexID;
    g_moduleDataPtr->mDepthTargetTexID = targetDepthTexID;
    g_moduleDataPtr->mDepthTargetRbo = targetDepthRbo;
    g_moduleDataPtr->mEnableClearColor = clearColor;
    g_moduleDataPtr->mModelMat4 = *(static_cast<glm::mat4*>(modelMat4));
    g_moduleDataPtr->mViewMat4 = *(static_cast<glm::mat4*>(viewMat4));
    g_moduleDataPtr->mProjectionMat4 = *(static_cast<glm::mat4*>(projectionMat4));
    g_moduleDataPtr->mViewPosition = *(static_cast<glm::vec3*>(viewPositionVec3));

    return g_modulePtr->grProcessModule();

}

void grReleasePbrRender()
{
    delete g_modulePtr;
    g_modulePtr = nullptr;
    delete g_moduleDataPtr;
    g_moduleDataPtr = nullptr;
}
