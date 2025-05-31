
#pragma once
#include "cppUtils.h"
#include "YYProgram.h"

class YYGLModule3D {
public:
    struct YYGLModule3DData {
        struct ProgramShaderPathPackage
        {
            std::string programName;
            std::string vsFilePath;
            std::string fsFilePath;
            std::string gsFilePath;
        };
        // Asset Manager
        void* mAssetMgr = nullptr;
        // Shader Files
        std::vector<ProgramShaderPathPackage> mProgramPathPackages;

        // Target Res Date
        int mLowLeftX = 0;
        int mLowLeftY = 0;
        int mTargetWidth = 0;
        int mTargetHeight = 0;

        // MVP data
        glm::mat4 mProjectionMat4; // default is identity matrix
        glm::mat4 mViewMat4;
        std::vector<glm::mat4> mModelMat4s;

        // camera data
        glm::vec3 mCameraPos = glm::vec3(0.0f, 0.0f, 0.0f);

        // Feature toggle
        bool mEnableDepthTest = false;
        GLenum mDepthFunc = GL_LESS;
        bool mEnableStencil = false;
        bool mEnableCullFace = false;
        GLenum mCullFaceFront = GL_CCW;

        // Clear
        bool mEnableClearColor = false;
        glm::vec4 mClearColor = glm::vec4(0.2f, 0.3f, 0.3f, 1.0f);
        float mClearDepth = 1.0f;

        // outSide target texID
        GLuint mColorTargetTexID = 0;
        GLuint mDepthTargetRbo = 0;
        GLuint mDepthTargetTexID = 0;

        std::vector<std::string> modelsLoadPath;
        bool mLoadTexFlipV = true;
        bool mRegenerateResources = false;
    };
    YYGLModule3D(std::shared_ptr<YYGLModule3DData> data) : mModuleData(data) {YYLog::D("YYGLModule3D create");}
    virtual std::shared_ptr<YYGLModule3DData> getYYGLModuleData() { return mModuleData; }
    virtual ~YYGLModule3D() { glDeleteFramebuffers(1, &mFbo); YYLog::D("YYGLModule3D destroy"); }
    // Init VAO and Program
    virtual void grInitModule() {
        // create program
        for (auto package : mModuleData->mProgramPathPackages)
        {
            std::shared_ptr<YYProgram> program = std::make_shared<YYProgram>(mModuleData->mAssetMgr);
            if (package.gsFilePath.empty())
            {
                program->LoadVertFrag(package.vsFilePath, package.fsFilePath);
            }
            else
            {
                program->LoadVertGeomFrag(package.vsFilePath, package.gsFilePath, package.fsFilePath);
            }
            if (!program) {
                YYLog::E("program create error !");
            }
            CHECK_GL_ERROR
                mPrograms[package.programName] = program;
        }
        

        for (auto objPath : mModuleData->modelsLoadPath)
        {
            mModels[objPath] = std::make_shared<YYGLModelData::AssimpModel>(objPath, mModuleData->mLoadTexFlipV);
        }
    };

    // return target texID
    virtual unsigned int grProcessModule() {
        if (mModuleData->mRegenerateResources)
        {
            deleteResources();
            generateResources();
        }
        if (mModuleData->mEnableCullFace) {
            glFrontFace(mModuleData->mCullFaceFront);
            glEnable(GL_CULL_FACE);
        } else {
            glDisable(GL_CULL_FACE);
        }
        if (mModuleData->mEnableDepthTest) {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(mModuleData->mDepthFunc);
        } else {
            glDisable(GL_DEPTH_TEST);
        }
        mModuleData->mEnableStencil ?  glEnable(GL_STENCIL_TEST) : glDisable(GL_STENCIL_TEST);

        if (mModuleData->mColorTargetTexID != 0) {
            glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mModuleData->mColorTargetTexID, 0);
            if (mModuleData->mDepthTargetTexID != 0) {
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mModuleData->mDepthTargetTexID, 0);
            } else if (mModuleData->mDepthTargetRbo != 0) {
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mModuleData->mDepthTargetRbo);
            }
            GLenum res = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            YY_DEMO_ASSERT(res == GL_FRAMEBUFFER_COMPLETE);
            CHECK_GL_ERROR
        } else
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        glViewport(mModuleData->mLowLeftX, mModuleData->mLowLeftY, mModuleData->mTargetWidth, mModuleData->mTargetHeight);



        if (mModuleData->mEnableClearColor || mModuleData->mEnableDepthTest) {
            glClearColor(mModuleData->mClearColor[0],
                        mModuleData->mClearColor[1],
                        mModuleData->mClearColor[2],
                        mModuleData->mClearColor[3]);
            glClearDepthf(mModuleData->mClearDepth);
            GLenum clearFlag = mModuleData->mEnableClearColor ? GL_COLOR_BUFFER_BIT : 0;
            clearFlag = mModuleData->mEnableDepthTest ? (clearFlag | GL_DEPTH_BUFFER_BIT) : clearFlag;
            glClear(clearFlag);
        }

        return mModuleData->mColorTargetTexID;
    }

    YYGLModule3D(const YYGLModule3D&) = delete;
    YYGLModule3D& operator=(const YYGLModule3D&) = delete;

protected:
    unsigned int mFbo = 0;
    std::shared_ptr<YYGLModule3DData> mModuleData;
    std::unordered_map<std::string, std::shared_ptr<YYProgram>> mPrograms;
    std::unordered_map<std::string, std::shared_ptr<YYGLModelData::AssimpModel>> mModels;
    // create resources
    virtual void generateResources()
    {
        YYLog::D("YYGLModule3D generateResources");
        // generate FBO
        glGenFramebuffers(1, &mFbo);
    }
    // delete resources
    virtual void deleteResources()
    {
        YYLog::D("YYGLModule3D deleteResources");
        glDeleteFramebuffers(1, &mFbo);
    }
};
