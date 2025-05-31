//
// Created by yuyao on 2024/2/18.
//

#include "yyModelRender.h"

namespace {
    class ModelRender : public YYGLModule3D {
    public:
        explicit ModelRender(std::shared_ptr<YYGLModule3DData> data) : YYGLModule3D(data) { YYLog::D("ModelRender create"); }
        ~ModelRender() { YYLog::D("ModelRender destroy"); };

        void grInitModule() override {
            YYGLModule3D::grInitModule();
        }

        unsigned int grProcessModule() override {
            YYGLModule3D::grProcessModule();
            std::shared_ptr<YYProgram> program = mPrograms[mModuleData->mProgramPathPackages[0].programName];
            program->Bind();
            program->SetUniform("projection", mModuleData->mProjectionMat4);
            program->SetUniform("view", mModuleData->mViewMat4);
            for (int i = 0; i < mModuleData->mModelMat4s.size(); ++i)
            {
                program->SetUniform("model", mModuleData->mModelMat4s[i]);
                mModels[mModuleData->modelsLoadPath[0]]->Draw(program->GetProgram()); // only draw first model
            }
            return mModuleData->mColorTargetTexID;
        }
        ModelRender(const ModelRender&) = delete;
        ModelRender& operator=(const ModelRender&) = delete;

    private:
    };
}

std::shared_ptr<YYGLModule3D> yyInitModelRender(void* assetMgr, const char* modelPath, bool flipV)
{
    std::shared_ptr<YYGLModule3D::YYGLModule3DData> data = std::make_shared<YYGLModule3D::YYGLModule3DData>();
    data->mAssetMgr = assetMgr;
    data->modelsLoadPath.push_back(std::string(modelPath));
    data->mLoadTexFlipV = flipV;
    YYGLModule3D::YYGLModule3DData::ProgramShaderPathPackage package = {
        "ModelRender",
        "shaders/3D/model_loading_test.vs",
        "shaders/3D/model_loading_test.fs",
        ""
    };
    data->mProgramPathPackages.push_back(package);
    data->mEnableDepthTest = true;
    data->mEnableClearColor = true;
    data->mEnableCullFace = true;
    std::shared_ptr<ModelRender> modulePtr = std::make_shared<ModelRender>(data);
    modulePtr->grInitModule();
    return modulePtr;
}


void yyProcessModelRender(const std::shared_ptr<YYGLModule3D> modulePtr,
                         unsigned int targetColorTexID,
                         unsigned int targetDepthTexID,
                         int targetWidth,
                         int targetHeight,
                         glm::mat4 projectionMat,
                         glm::mat4 viewMat,
                         std::vector<glm::mat4> modelMats)
{
    std::shared_ptr<YYGLModule3D::YYGLModule3DData> data = modulePtr->getYYGLModuleData();
    assert(data != nullptr);
    data->mRegenerateResources = ((targetColorTexID != 0) && ((targetWidth != data->mTargetWidth) || (targetHeight != data->mTargetHeight)));
    data->mTargetWidth = targetWidth;
    data->mTargetHeight = targetHeight;
    data->mColorTargetTexID = targetColorTexID;
    data->mDepthTargetTexID = targetDepthTexID;
    data->mProjectionMat4 = projectionMat;
    data->mModelMat4s = modelMats;
    data->mViewMat4 = viewMat;
    modulePtr->grProcessModule();
}


