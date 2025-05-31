//
// Copyright (c) 2022-present, YYAL.  All rights reserved.
// Created by Yaolong Yu on 2022/8/11.
//

#include "irradianceMap.h"
#include "miscUtils.h"
#include "YYGLModule.hpp"

namespace {
    struct IrradianceMapModuleData : YYGLModuleData {
        std::vector<glm::mat4> mCaptureViews = {
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
        };
    };

    class IrradianceMapModule : public YYGLModule {
    public:
        explicit IrradianceMapModule(const IrradianceMapModuleData &mModuleData) : YYGLModule(mModuleData) {}
        ~IrradianceMapModule() override = default;

         void grInitModule() override {
             YYGLModule::grInitModule();
             glUseProgram(mProgram);
             glUtils::setUniformInt(mProgram, "environmentMap", 0);
             glUtils::setUniformMat4(mProgram, "projection", &mModuleData.mProjectionMat4[0][0]);
             glUseProgram(0);
        }

        unsigned int grProcessModule() override {
            glEnable(GL_CULL_FACE);
            glFrontFace(GL_CW);
            YY_DEMO_ASSERT(mModuleData.mEnableDepthTest == true)
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_STENCIL_TEST);


            glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
            glClearDepthf(1.0f);

            glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
            YY_DEMO_ASSERT(mModuleData.mDepthTargetRbo != 0)
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mModuleData.mDepthTargetRbo);

            glViewport(mModuleData.mLowLeftX, mModuleData.mLowLeftY, mModuleData.mTargetWidth, mModuleData.mTargetHeight);

            glUseProgram(mProgram);
            glBindVertexArray(mVao);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(mModuleData.mSourceTexArray[0].texTarget, mModuleData.mSourceTexArray[0].texID);
            CHECK_GL_ERROR

            for (unsigned int i = 0; i < 6; ++i)
            {
                mFrameCnt++;
                glUtils::setUniformMat4(mProgram, "view", &(dynamic_cast<const IrradianceMapModuleData&>(mModuleData).mCaptureViews[i][0][0]));
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,  mModuleData.mColorTargetTexID, 0);
                GLenum res = glCheckFramebufferStatus(GL_FRAMEBUFFER);
                YY_DEMO_ASSERT(res == GL_FRAMEBUFFER_COMPLETE);

                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glDrawArrays(GL_TRIANGLES, 0, mModuleData.mVertexOrIndexNum);
            }
            CHECK_GL_ERROR

            glBindTexture(GL_TEXTURE_2D, 0);
            glBindVertexArray(0);
            glUseProgram(0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            return mModuleData.mColorTargetTexID;
         }

        IrradianceMapModule(const IrradianceMapModule&) = delete;
        IrradianceMapModule& operator=(const IrradianceMapModule&) = delete;

    private:
    };

    IrradianceMapModule* g_modulePtr = nullptr;
    IrradianceMapModuleData* g_moduleDataPtr = nullptr;

}

// interface
void grInitIrradianceMap(void* assetMgr)
{
    g_moduleDataPtr = new IrradianceMapModuleData();
    YY_DEMO_ASSERT(g_moduleDataPtr != nullptr)
    g_modulePtr = new IrradianceMapModule(*g_moduleDataPtr);
    YY_DEMO_ASSERT(g_modulePtr != nullptr)
    g_moduleDataPtr->mAssetMgr = assetMgr,
    g_moduleDataPtr->mVsFileName = "shaders/IBL/irradianceMap/irradianceMap.vs";
    g_moduleDataPtr->mFsFileName = "shaders/IBL/irradianceMap/irradianceMap.fs";
    g_moduleDataPtr->mModelDataPtr = ModelData::CUBE_POS3_NORMAL3_TEXCOOR2;
    g_moduleDataPtr->mModelDataLen = 288;
    g_moduleDataPtr->mAttributesSizeArray = {3, 3, 2};
    g_moduleDataPtr->mVertexOrIndexNum = 36;

    g_moduleDataPtr->mProjectionMat4 = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    g_moduleDataPtr->mSourceTexArray.resize(1);
    g_moduleDataPtr->mEnableDepthTest = true;
    g_moduleDataPtr->mCullFaceFront = GL_CW;
    g_modulePtr->grInitModule();


}

unsigned int grProcessIrradianceMap(unsigned int srcColorTexID,
                                     unsigned int targetColorTexID,
                                     unsigned int targetDepthRbo,
                                     int targetWidth,
                                     int targetHeight)
{

    g_moduleDataPtr->mSourceTexArray[0].texID = srcColorTexID;
    g_moduleDataPtr->mSourceTexArray[0].texTarget = GL_TEXTURE_CUBE_MAP;
    g_moduleDataPtr->mColorTargetTexID = targetColorTexID;
    g_moduleDataPtr->mDepthTargetRbo = targetDepthRbo;
    g_moduleDataPtr->mTargetWidth = targetWidth;
    g_moduleDataPtr->mTargetHeight = targetHeight;
    return g_modulePtr->grProcessModule();
}

void grReleaseIrradianceMap()
{
    delete g_modulePtr;
    g_modulePtr = nullptr;
    delete g_moduleDataPtr;
    g_moduleDataPtr = nullptr;
}
