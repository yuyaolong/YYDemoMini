//
// Copyright (c) 2022-present, YYAL.  All rights reserved.
// Created by Yaolong Yu on 2022/8/11.
//

#include "environmentMap.h"
#include "miscUtils.h"
#include "YYGLModule.hpp"

namespace {
    class EnvironmentMapModule : public YYGLModule {
    public:
        explicit EnvironmentMapModule(const YYGLModuleData &mModuleData) : YYGLModule(mModuleData) {}
        ~EnvironmentMapModule() override = default;

        void grInitModule() override {
            YYGLModule::grInitModule();
            glUseProgram(mProgram);
            glUniform1i(glGetUniformLocation(mProgram, "environmentMap"), 0);
            glUseProgram(0);
        }

        unsigned int grProcessModule() override {
            glUseProgram(mProgram);
            glUniformMatrix4fv(glGetUniformLocation(mProgram, "projection"), 1, GL_FALSE, &(mModuleData.mProjectionMat4[0][0]));
            glUniformMatrix4fv(glGetUniformLocation(mProgram, "view"), 1, GL_FALSE, &(mModuleData.mViewMat4[0][0]));
            glUseProgram(0);
            return YYGLModule::grProcessModule();
        }
        EnvironmentMapModule(const EnvironmentMapModule&) = delete;
        EnvironmentMapModule& operator=(const EnvironmentMapModule&) = delete;

    private:

    };

    YYGLModuleData* g_moduleDataPtr = nullptr;
    EnvironmentMapModule* g_modulePtr = nullptr;

}


void grInitEnvironmentMap(void* assetMgr)
{
    g_moduleDataPtr = new YYGLModuleData();
    YY_DEMO_ASSERT(g_moduleDataPtr != nullptr)
    g_modulePtr = new EnvironmentMapModule(*g_moduleDataPtr);
    YY_DEMO_ASSERT(g_modulePtr != nullptr)
    g_moduleDataPtr->mAssetMgr = assetMgr;
    g_moduleDataPtr->mVsFileName = "shaders/IBL/environmentMap/environmentMap.vs";
    g_moduleDataPtr->mFsFileName = "shaders/IBL/environmentMap/environmentMap.fs";
    g_moduleDataPtr->mModelDataPtr = ModelData::CUBE_POS3_NORMAL3_TEXCOOR2;
    g_moduleDataPtr->mModelDataLen = 288;
    g_moduleDataPtr->mAttributesSizeArray = {3, 3, 2};
    g_moduleDataPtr->mVertexOrIndexNum = 36;
    g_moduleDataPtr->mEnableDepthTest = true;
    g_moduleDataPtr->mDepthFunc = GL_LEQUAL; // depth for skybox is always 1.0
    g_moduleDataPtr->mEnableClearColor = true;
    g_moduleDataPtr->mClearColor = {0.0f, 1.0f, 0.0f, 1.0f};
    g_moduleDataPtr->mEnableCullFace = true;
    g_moduleDataPtr->mCullFaceFront = GL_CW; // inside-out cube triangle is CW vertices order
    g_moduleDataPtr->mSourceTexArray.resize(1);
    g_modulePtr->grInitModule();

}

// texture is cubeMap type
unsigned int grProcessEnvironmentMap(unsigned int showTexID,
                             int screenWidth,
                             int screenHeight,
                             unsigned int targetColorTexID,
                             unsigned int targetDepthTexID,
                             unsigned int targetDepthRbo,
                             void* viewMat4,
                             void* projectionMat4)
{
    YY_DEMO_ASSERT(g_modulePtr != nullptr)
    g_moduleDataPtr->mSourceTexArray[0].texID = showTexID;
    g_moduleDataPtr->mSourceTexArray[0].texTarget = GL_TEXTURE_CUBE_MAP;
    g_moduleDataPtr->mColorTargetTexID = targetColorTexID;
    g_moduleDataPtr->mDepthTargetTexID = targetDepthTexID;
    g_moduleDataPtr->mDepthTargetRbo = targetDepthRbo;
    g_moduleDataPtr->mTargetWidth = screenWidth;
    g_moduleDataPtr->mTargetHeight = screenHeight;
    g_moduleDataPtr->mViewMat4 = *(static_cast<glm::mat4*>(viewMat4));
    g_moduleDataPtr->mProjectionMat4 = *(static_cast<glm::mat4*>(projectionMat4));
    return g_modulePtr->grProcessModule();
}

void grReleaseEnvironmentMap()
{
    delete g_modulePtr;
    g_modulePtr = nullptr;
    delete g_moduleDataPtr;
    g_moduleDataPtr = nullptr;
}
