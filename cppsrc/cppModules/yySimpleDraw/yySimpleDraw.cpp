//
// Created by yuyao on 2024/2/18.
//

#include "yySimpleDraw.h"
// #include "YYGLModelData.h"
namespace {
    class SimpleDrawWithDepth : public YYGLModule {
    public:
        explicit SimpleDrawWithDepth(std::shared_ptr<YYGLModuleData> data) : YYGLModule(data) { YYLog::D("SimpleDrawWithDepth create"); }
        ~SimpleDrawWithDepth() { YYLog::D("SimpleDrawWithDepth destroy"); };

        void grInitModule() override {
            YYGLModule::grInitModule();
            mProgram->Bind();
            mProgram->SetUniform("srcTex", 0);
            glUseProgram(0);
        }

        unsigned int grProcessModule() override {

            return YYGLModule::grProcessModule();
        }
        SimpleDrawWithDepth(const SimpleDrawWithDepth&) = delete;
        SimpleDrawWithDepth& operator=(const SimpleDrawWithDepth&) = delete;

    private:
    };
}

std::shared_ptr<YYGLModule> yyInitSimpleDraw(void* assetMgr, bool use2DArraySrcTex)
{
    std::shared_ptr<YYGLModule::YYGLModuleData> data = std::make_shared<YYGLModule::YYGLModuleData>();
    data->mAssetMgr = assetMgr;
    data->mVsFileName = "shaders/2D/passthrough.vs";
    if (use2DArraySrcTex) {
        data->mFsFileName = "shaders/2D/passthrough2DArray.fs";
    } else {
        data->mFsFileName = "shaders/2D/passthrough.fs";
    }
    data->mModelDataPtr = YYGLModelData::RECTANGLE_POS2_TEXCOOR2;
    data->mModelDataLen = 24;
    data->mAttributesSizeArray = {2, 2};
    data->mSourceTexArray.resize(1);

    std::shared_ptr<YYGLModule> modulePtr = std::make_shared<SimpleDrawWithDepth>(data);
    modulePtr->grInitModule();

    return modulePtr;
}


void yyProcessSimpleDraw(const std::shared_ptr<YYGLModule> modulePtr,
                         unsigned int srcTexID,
                         unsigned int targetColorTexID,
                         unsigned int targetDepthTexID,
                         int targetWidth,
                         int targetHeight,
                         bool enableDepth)
{
    std::shared_ptr<YYGLModule::YYGLModuleData> data = modulePtr->getYYGLModuleData();
    assert(data != nullptr);
    data->mSourceTexArray[0].texID = srcTexID;
    data->mTargetWidth = targetWidth;
    data->mTargetHeight = targetHeight;
    data->mColorTargetTexID = targetColorTexID;
    data->mDepthTargetTexID = targetDepthTexID;
    data->mEnableDepthTest = enableDepth;
    modulePtr->grProcessModule();
}