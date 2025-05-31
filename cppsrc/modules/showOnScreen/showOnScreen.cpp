//
// Copyright (c) 2022-present, YYAL.  All rights reserved.
// Created by Yaolong Yu on 2022/8/11.
//

#include "showOnScreen.h"
#include "miscUtils.h"
#include "YYGLModule.hpp"

namespace {
    class ShowOnScreenModule : public YYGLModule {
    public:
        explicit ShowOnScreenModule(const YYGLModuleData &mModuleData) : YYGLModule(mModuleData) {}
        ~ShowOnScreenModule() override = default;

        void grInitModule() override {
            YYGLModule::grInitModule();
            glUseProgram(mProgram);
            glUniform1i(glGetUniformLocation(mProgram, "srcTex"), 0);
            glUseProgram(0);
        }

        unsigned int grProcessModule() override {

            return YYGLModule::grProcessModule();
        }
        ShowOnScreenModule(const ShowOnScreenModule&) = delete;
        ShowOnScreenModule& operator=(const ShowOnScreenModule&) = delete;

    private:

    };

    YYGLModuleData* g_moduleDataPtr = nullptr;
    ShowOnScreenModule* g_modulePtr = nullptr;

}


void grInitShowOnScreen(void* assetMgr)
{
    g_moduleDataPtr = new YYGLModuleData();
    YY_DEMO_ASSERT(g_moduleDataPtr != nullptr)
    g_modulePtr = new ShowOnScreenModule(*g_moduleDataPtr);
    YY_DEMO_ASSERT(g_modulePtr != nullptr)
    g_moduleDataPtr->mAssetMgr = assetMgr;
    g_moduleDataPtr->mVsFileName = "shaders/2D/passthrough_es.vs";
    g_moduleDataPtr->mFsFileName = "shaders/2D/passthrough_es.fs";
    g_moduleDataPtr->mModelDataPtr = ModelData::RECTANGLE_POS2_TEXCOOR2;
    g_moduleDataPtr->mModelDataLen = 24;
    g_moduleDataPtr->mAttributesSizeArray = {2, 2};
    g_moduleDataPtr->mSourceTexArray.resize(1);
    g_moduleDataPtr->mColorTargetTexID = 0;
    g_moduleDataPtr->mDepthTargetRbo = 0;
    g_moduleDataPtr->mDepthTargetTexID = 0;
    g_modulePtr->grInitModule();

}

void grProcessShowOnScreen(unsigned int showTexID,
                           int lowLeftX,
                           int lowLeftY,
                           int screenWidth,
                           int screenHeight)
{
    YY_DEMO_ASSERT(g_modulePtr != nullptr)
    g_moduleDataPtr->mSourceTexArray[0].texID = showTexID;
    g_moduleDataPtr->mLowLeftX = lowLeftX;
    g_moduleDataPtr->mLowLeftY = lowLeftY;
    g_moduleDataPtr->mTargetWidth = screenWidth;
    g_moduleDataPtr->mTargetHeight = screenHeight;
    g_modulePtr->grProcessModule();
}

void grReleaseShowOnScreen()
{
    delete g_modulePtr;
    g_modulePtr = nullptr;
    delete g_moduleDataPtr;
    g_moduleDataPtr = nullptr;
}
