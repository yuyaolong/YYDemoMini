//
// Created by yuyao on 2023/12/24.
//

#pragma once
#include "cppUtils.h"
#include "YYProgram.h"

class YYGLModule {
public:
    struct YYTextureData {
        GLuint texID = 0;
        GLenum texTarget = GL_TEXTURE_2D;
        int texUniformIdx = 0;
        const char* texUName = nullptr;
    };

    struct YYGLModuleData {
        virtual ~YYGLModuleData() = default;
        // Asset Manager
        void* mAssetMgr = nullptr;

        // Shader Files
        const char * mVsFileName = nullptr;
        const char * mFsFileName = nullptr;

        // VBO Model Data
        const float* mModelDataPtr = nullptr;
        int mModelDataLen = 24;

        // EBO Modle Data
        const unsigned int* mElementDataPtr = nullptr;
        int mElementDataLen = 0;

        // Attributes Data
        std::vector<int> mAttributesSizeArray{2, 2};

        // Source Tex Array
        std::vector<YYTextureData> mSourceTexArray;

        // Target Res Date
        int mLowLeftX = 0;
        int mLowLeftY = 0;
        int mTargetWidth = 0;
        int mTargetHeight = 0;

        // MVP data
        glm::mat4 mProjectionMat4; // default is identity matrix
        glm::mat4 mViewMat4;
        glm::mat4 mModelMat4;

        // Vertices Data
        int mVertexOrIndexNum = 6;

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

        // Draw type
        bool mEnableElementDraw = false;
        GLenum mDrawType = GL_TRIANGLES;
    };
    YYGLModule(std::shared_ptr<YYGLModuleData> data) : mModuleData(data) {}
    virtual std::shared_ptr<YYGLModuleData> getYYGLModuleData() { return mModuleData; }
    virtual ~YYGLModule() {
        glDeleteBuffers(1, &mVbo);
        glDeleteBuffers(1, &mEbo);
        glDeleteVertexArrays(1, &mVao);
        glDeleteFramebuffers(1, &mFbo);
    }
    // Init VAO and Program
    virtual void grInitModule() {
        // generate FBO
        glGenFramebuffers(1, &mFbo);
        // generate vao and vbo
        glGenVertexArrays(1, &mVao); 
        glGenBuffers(1, &mVbo); 
        glBindVertexArray(mVao); 
        glBindBuffer(GL_ARRAY_BUFFER, mVbo); 
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mModuleData->mModelDataLen, mModuleData->mModelDataPtr, GL_STATIC_DRAW);
        int accumulateBase = 0;
        int totalSize = 0;
        for (int num : mModuleData->mAttributesSizeArray) {
            totalSize += num;
        }
        for (int i = 0; i < mModuleData->mAttributesSizeArray.size(); ++i)
        {
            glEnableVertexAttribArray(i);
            glVertexAttribPointer(i, mModuleData->mAttributesSizeArray[i], GL_FLOAT, GL_FALSE, totalSize * sizeof(float), (void *) (accumulateBase * sizeof(float)));
            accumulateBase += mModuleData->mAttributesSizeArray[i];
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        if (mModuleData->mEnableElementDraw) {
            YY_DEMO_ASSERT((mModuleData->mElementDataPtr != nullptr) && (mModuleData->mElementDataLen != 0));
            glGenBuffers(1, &mEbo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEbo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, mModuleData->mElementDataLen * sizeof(unsigned int), mModuleData->mElementDataPtr, GL_STATIC_DRAW);
        }
        glBindVertexArray(0);
        CHECK_GL_ERROR

        // create program
        mProgram = std::make_shared<YYProgram>(mModuleData->mAssetMgr);
        if (!mProgram->LoadVertFrag(mModuleData->mVsFileName, mModuleData->mFsFileName)) {
            YYLog::E("program create error !");
        }
        CHECK_GL_ERROR
    };

    // return target texID
    virtual unsigned int grProcessModule() {
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

        mProgram->Bind();
        glBindVertexArray(mVao);
        for (auto tex : mModuleData->mSourceTexArray) {
            glActiveTexture(GL_TEXTURE0 + tex.texUniformIdx);
            glBindTexture(tex.texTarget, tex.texID);
        }

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

        if (mModuleData->mEnableElementDraw) {
            glDrawElements(mModuleData->mDrawType, mModuleData->mVertexOrIndexNum, GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(mModuleData->mDrawType, 0, mModuleData->mVertexOrIndexNum);
        }

        for (auto tex : mModuleData->mSourceTexArray) {
            glBindTexture(tex.texTarget, 0);
        }
        glBindVertexArray(0);
        glUseProgram(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return mModuleData->mColorTargetTexID;
    }

    YYGLModule(const YYGLModule&) = delete;
    YYGLModule& operator=(const YYGLModule&) = delete;




protected:
    unsigned int mFbo = 0;
    unsigned int mVao = 0;
    unsigned int mVbo = 0;
    unsigned int mEbo = 0;
    std::shared_ptr<YYProgram> mProgram;
    std::shared_ptr<YYGLModuleData> mModuleData;
    unsigned int mFrameCnt = 0;
};
