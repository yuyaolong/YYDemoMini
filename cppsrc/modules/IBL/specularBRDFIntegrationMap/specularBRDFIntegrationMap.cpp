//
// Copyright (c) 2022-present, YYAL.  All rights reserved.
// Created by Yaolong Yu on 2022/8/11.
//

#include "specularBRDFIntegrationMap.h"
#include "miscUtils.h"
#include "YYGLModule.hpp"

namespace {
    class SpecularBRDFIntegrationMapModule : public YYGLModule {
    public:
        explicit SpecularBRDFIntegrationMapModule(const YYGLModuleData &mModuleData) : YYGLModule(mModuleData) {}
        ~SpecularBRDFIntegrationMapModule() override = default;

        void grInitModule() override {
            YYGLModule::grInitModule();
        }

        unsigned int grProcessModule() override {
            return YYGLModule::grProcessModule();
        }
        SpecularBRDFIntegrationMapModule(const SpecularBRDFIntegrationMapModule&) = delete;
        SpecularBRDFIntegrationMapModule& operator=(const SpecularBRDFIntegrationMapModule&) = delete;

    private:

    };

    YYGLModuleData* g_moduleDataPtr = nullptr;
    SpecularBRDFIntegrationMapModule* g_modulePtr = nullptr;

}


void grInitSpecularBRDFIntegrationMap(void* assetMgr)
{
    g_moduleDataPtr = new YYGLModuleData();
    YY_DEMO_ASSERT(g_moduleDataPtr != nullptr)
    g_modulePtr = new SpecularBRDFIntegrationMapModule(*g_moduleDataPtr);
    YY_DEMO_ASSERT(g_modulePtr != nullptr)
    g_moduleDataPtr->mAssetMgr = assetMgr;
    g_moduleDataPtr->mVsFileName = "shaders/IBL/specularBRDFIntegrationMap/specularBRDFIntegrationMap.vs";
    g_moduleDataPtr->mFsFileName = "shaders/IBL/specularBRDFIntegrationMap/specularBRDFIntegrationMap.fs";
    g_moduleDataPtr->mModelDataPtr = ModelData::RECTANGLE_POS2_TEXCOOR2;
    g_moduleDataPtr->mModelDataLen = 24;
    g_moduleDataPtr->mAttributesSizeArray = {2, 2};
    g_moduleDataPtr->mVertexOrIndexNum = 6;
    g_moduleDataPtr->mEnableDepthTest = true;
    g_moduleDataPtr->mEnableClearColor = true;
    g_moduleDataPtr->mClearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    g_moduleDataPtr->mEnableCullFace = true;
    g_moduleDataPtr->mCullFaceFront = GL_CCW;
    g_moduleDataPtr->mSourceTexArray.resize(0);
    g_modulePtr->grInitModule();

}

// texture is cubeMap type
unsigned int grProcessSpecularBRDFIntegrationMap(int screenWidth,
                                                 int screenHeight,
                                                 unsigned int targetColorTexID,
                                                 unsigned int targetDepthRbo)
{
    YY_DEMO_ASSERT(g_modulePtr != nullptr)
    g_moduleDataPtr->mColorTargetTexID = targetColorTexID;
    g_moduleDataPtr->mDepthTargetRbo = targetDepthRbo;
    g_moduleDataPtr->mTargetWidth = screenWidth;
    g_moduleDataPtr->mTargetHeight = screenHeight;
    return g_modulePtr->grProcessModule();
}

void grReleaseSpecularBRDFIntegrationMap()
{
    delete g_modulePtr;
    g_modulePtr = nullptr;
    delete g_moduleDataPtr;
    g_moduleDataPtr = nullptr;
}
