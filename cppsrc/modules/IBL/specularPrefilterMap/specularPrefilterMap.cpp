//
// Copyright (c) 2022-present, YYAL.  All rights reserved.
// Created by Yaolong Yu on 2023/1/11.
//

#include "specularPrefilterMap.h"
#include "miscUtils.h"
#include "YYGLModule.hpp"

namespace {
    struct SpecularPrefilterMapModuleData : YYGLModuleData {
        std::vector<glm::mat4> mCaptureViews = {
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
        };
    };

    class SpecularPrefilterMapModule : public YYGLModule {
    public:
        explicit SpecularPrefilterMapModule(const SpecularPrefilterMapModuleData &mModuleData) : YYGLModule(mModuleData) {}
        ~SpecularPrefilterMapModule() override = default;

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

            int maxMipLevels = int(log2(std::max(mModuleData.mTargetWidth, mModuleData.mTargetHeight))) + 1;
            for (int mip = 0; mip < maxMipLevels; ++mip)
            {
                int mipWidth = static_cast<int>(mModuleData.mTargetWidth * std::pow(0.5, mip));
                int mipHeight = static_cast<int>(mModuleData.mTargetHeight * std::pow(0.5, mip));
                glBindRenderbuffer(GL_RENDERBUFFER, mModuleData.mDepthTargetRbo);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mModuleData.mDepthTargetRbo);
                glBindRenderbuffer(GL_RENDERBUFFER, 0);


                glViewport(0, 0, mipWidth, mipHeight);

                glUseProgram(mProgram);

                // update roughness
                float roughness = (float)mip / (float)(maxMipLevels - 1);
                glUtils::setUniformFloat(mProgram, "roughness", roughness);

                glBindVertexArray(mVao);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(mModuleData.mSourceTexArray[0].texTarget, mModuleData.mSourceTexArray[0].texID);
                CHECK_GL_ERROR

                for (unsigned int i = 0; i < 6; ++i)
                {
                    glUtils::setUniformMat4(mProgram, "view", &(dynamic_cast<const SpecularPrefilterMapModuleData&>(mModuleData).mCaptureViews[i][0][0]));
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,  mModuleData.mColorTargetTexID, mip);
                    GLenum res = glCheckFramebufferStatus(GL_FRAMEBUFFER);
                    YY_DEMO_ASSERT(res == GL_FRAMEBUFFER_COMPLETE);

                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    glDrawArrays(GL_TRIANGLES, 0, mModuleData.mVertexOrIndexNum);
                }
                CHECK_GL_ERROR
            }


            glBindTexture(GL_TEXTURE_2D, 0);
            glBindVertexArray(0);
            glUseProgram(0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            return mModuleData.mColorTargetTexID;
         }

        SpecularPrefilterMapModule(const SpecularPrefilterMapModule&) = delete;
        SpecularPrefilterMapModule& operator=(const SpecularPrefilterMapModule&) = delete;

    private:
    };

    SpecularPrefilterMapModule* g_modulePtr = nullptr;
    SpecularPrefilterMapModuleData* g_moduleDataPtr = nullptr;

}

// interface
void grInitSpecularPrefilterMap(void* assetMgr)
{
    g_moduleDataPtr = new SpecularPrefilterMapModuleData();
    YY_DEMO_ASSERT(g_moduleDataPtr != nullptr)
    g_modulePtr = new SpecularPrefilterMapModule(*g_moduleDataPtr);
    YY_DEMO_ASSERT(g_modulePtr != nullptr)
    g_moduleDataPtr->mAssetMgr = assetMgr,
    g_moduleDataPtr->mVsFileName = "shaders/IBL/specularPrefilterMap/specularPrefilterMap.vs";
    g_moduleDataPtr->mFsFileName = "shaders/IBL/specularPrefilterMap/specularPrefilterMap.fs";
    g_moduleDataPtr->mModelDataPtr = ModelData::CUBE_POS3_NORMAL3_TEXCOOR2;
    g_moduleDataPtr->mModelDataLen = 288;
    g_moduleDataPtr->mAttributesSizeArray = {3, 3, 2};
    g_moduleDataPtr->mVertexOrIndexNum = 36;

    g_moduleDataPtr->mProjectionMat4 = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);;
    g_moduleDataPtr->mSourceTexArray.resize(1);
    g_moduleDataPtr->mEnableDepthTest = true;
    g_moduleDataPtr->mCullFaceFront = GL_CW;
    g_modulePtr->grInitModule();


}

unsigned int grProcessSpecularPrefilterMap(unsigned int srcColorTexID,
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

void grReleaseSpecularPrefilterMap()
{
    delete g_modulePtr;
    g_modulePtr = nullptr;
    delete g_moduleDataPtr;
    g_moduleDataPtr = nullptr;
}
