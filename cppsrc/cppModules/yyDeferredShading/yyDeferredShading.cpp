//
// Created by yuyao on 2024/2/18.
//

#include "yyDeferredShading.h"
// #include "YYGLModelData.h"
namespace {
    const std::string CubeModelPath = "3DModels/cube/cube.obj";
    const std::string NanosuitModelPath = "3DModels/nanosuit/nanosuit.obj";
    const std::string CubeBoxProgramName = "cubeBox";
    const std::string DeferredShadingProgramName = "defferredShading";
    const std::string GBufferProgramName = "gBuffer";
    class DeferredShading : public YYGLModule3D {
    public:
        explicit DeferredShading(std::shared_ptr<YYGLModule3DData> data) : YYGLModule3D(data) { YYLog::D("DeferredShading create"); }
        ~DeferredShading() { YYLog::D("DeferredShading destroy"); };

        void grInitModule() override {
            YYGLModule3D::grInitModule();
            std::shared_ptr<YYProgram> deferredShadingProgram = mPrograms[DeferredShadingProgramName];
            deferredShadingProgram->Bind();
            deferredShadingProgram->SetUniform("gPosition", 0);
            deferredShadingProgram->SetUniform("gNormal", 1);
            deferredShadingProgram->SetUniform("gAlbedoSpec", 2);
            glUseProgram(0);
            CHECK_GL_ERROR
        }

        unsigned int grProcessModule() override {
            if (mModuleData->mRegenerateResources)
            {
                deleteResources();
                generateResources();
            }
            if (mModuleData->mEnableCullFace) {
                glFrontFace(mModuleData->mCullFaceFront);
                glEnable(GL_CULL_FACE);
            }
            else {
                glDisable(GL_CULL_FACE);
            }
            if (mModuleData->mEnableDepthTest) {
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(mModuleData->mDepthFunc);
            }
            else {
                glDisable(GL_DEPTH_TEST);
            }
            CHECK_GL_ERROR
            // gbuffer pass
            glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);

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

            // test draw cube
            std::shared_ptr<YYProgram> gBufferProgram = mPrograms[GBufferProgramName];
            gBufferProgram->Bind();
            gBufferProgram->SetUniform("projection", mModuleData->mProjectionMat4);
            gBufferProgram->SetUniform("view", mModuleData->mViewMat4);
            // glm::vec3 lightColor(0.5, 0.2, 0.2);
            // cubeProgram->SetUniform("lightColor", lightColor);
            for (int i = 0; i < mModuleData->mModelMat4s.size(); ++i)
            {
                gBufferProgram->SetUniform("model", mModuleData->mModelMat4s[i]);
                mModels[NanosuitModelPath]->Draw(gBufferProgram->GetProgram());
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            // lighting pass
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            std::shared_ptr<YYProgram> deferredShadingProgram = mPrograms[DeferredShadingProgramName];
            deferredShadingProgram->Bind();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gPoitionTex);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, gNormalTex);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, gAlbedoSpecTex);

            // send light relevant uniforms
            for (unsigned int i = 0; i < mLightPositions.size(); i++)
            {
                deferredShadingProgram->SetUniform("lights[" + std::to_string(i) + "].Position", mLightPositions[i]);
                deferredShadingProgram->SetUniform("lights[" + std::to_string(i) + "].Color", mLightColors[i]);
                // update attenuation parameters and calculate radius
                const float constant = 1.0f; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
                const float linear = 0.7f;
                const float quadratic = 1.8f;
                deferredShadingProgram->SetUniform("lights[" + std::to_string(i) + "].Linear", linear);
                deferredShadingProgram->SetUniform("lights[" + std::to_string(i) + "].Quadratic", quadratic);
                // then calculate radius of light volume/sphere
                const float maxBrightness = std::fmaxf(std::fmaxf(mLightColors[i].r, mLightColors[i].g), mLightColors[i].b);
                float radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * maxBrightness))) / (2.0f * quadratic);
                deferredShadingProgram->SetUniform("lights[" + std::to_string(i) + "].Radius", radius);
            }
            deferredShadingProgram->SetUniform("viewPos", mModuleData->mCameraPos);
            glBindVertexArray(mQuadVAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindVertexArray(0);

            // 2.5. copy content of geometry's depth buffer to default framebuffer's depth buffer
            // ----------------------------------------------------------------------------------
            glBindFramebuffer(GL_READ_FRAMEBUFFER, gBufferFBO);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
            // blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
            // the internal formats are implementation defined. This works on all of my systems, but if it doesn't on yours you'll likely have to write to the 		
            // depth buffer in another shader stage (or somehow see to match the default framebuffer's internal format with the FBO's internal format).
            glBlitFramebuffer(0, 0, mModuleData->mTargetWidth, mModuleData->mTargetHeight, 0, 0, mModuleData->mTargetWidth, mModuleData->mTargetHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // 3. render lights on top of scene
            // --------------------------------
            std::shared_ptr<YYProgram> cubeProgram = mPrograms[CubeBoxProgramName];
            cubeProgram->Bind();
            cubeProgram->SetUniform("projection", mModuleData->mProjectionMat4);
            cubeProgram->SetUniform("view", mModuleData->mViewMat4);
            glm::mat4 model = glm::mat4(1.0f);
            for (unsigned int i = 0; i < mLightPositions.size(); i++)
            {
                model = glm::mat4(1.0f);
                model = glm::translate(model, mLightPositions[i]);
                model = glm::scale(model, glm::vec3(0.125f));
                cubeProgram->SetUniform("model", model);
                cubeProgram->SetUniform("lightColor", mLightColors[i]);
                mModels[CubeModelPath]->Draw(cubeProgram->GetProgram());
            }

            glUseProgram(0);
            return 0;
        }
        DeferredShading(const DeferredShading&) = delete;
        DeferredShading& operator=(const DeferredShading&) = delete;

    private:
        unsigned int gBufferFBO = 0;
        unsigned int gPoitionTex = 0;
        unsigned int gNormalTex = 0;
        unsigned int gAlbedoSpecTex = 0;
        unsigned int gDepthRBO = 0;
        unsigned int NR_LIGHTS = 32;
        std::vector<glm::vec3> mLightPositions;
        std::vector<glm::vec3> mLightColors;

        unsigned int mQuadVAO = 0;
        unsigned int mQuadVBO = 0;

        virtual void generateResources()
        {
            YYLog::D("DeferredShading generateResources");
            glGenFramebuffers(1, &gBufferFBO);
            glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
            // - Position color buffer
            glGenTextures(1, &gPoitionTex);
            glBindTexture(GL_TEXTURE_2D, gPoitionTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, mModuleData->mTargetWidth, mModuleData->mTargetHeight, 0, GL_RGB, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPoitionTex, 0);
            // - Normal color buffer
            glGenTextures(1, &gNormalTex);
            glBindTexture(GL_TEXTURE_2D, gNormalTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, mModuleData->mTargetWidth, mModuleData->mTargetHeight, 0, GL_RGB, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormalTex, 0);
            // - Color + Specular color buffer
            glGenTextures(1, &gAlbedoSpecTex);
            glBindTexture(GL_TEXTURE_2D, gAlbedoSpecTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mModuleData->mTargetWidth, mModuleData->mTargetHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpecTex, 0);
            // - Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
            GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
            glDrawBuffers(3, attachments);
            // - Create and attach depth buffer (renderbuffer)
            GLuint gDepthRBO;
            glGenRenderbuffers(1, &gDepthRBO);
            glBindRenderbuffer(GL_RENDERBUFFER, gDepthRBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, mModuleData->mTargetWidth, mModuleData->mTargetHeight);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gDepthRBO);
            // - Finally check if framebuffer is complete
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                YYLog::E("Framebuffer not complete!");
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            CHECK_GL_ERROR

            // lighting info
            // -------------
            srand(13);
            for (unsigned int i = 0; i < NR_LIGHTS; i++)
            {
                // calculate slightly random offsets
                float yOffset = 1.0f;
                float zOffset = 1.0f;
                float xPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 3.0);
                float yPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 3.0);
                float zPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 3.0);
                mLightPositions.push_back(glm::vec3(xPos, yPos + yOffset, zPos + zOffset));
                // also calculate random color
                float rColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.)
                float gColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.)
                float bColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.)
                mLightColors.push_back(glm::vec3(rColor, gColor, bColor));
            }

            // lighting quad data
            float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            };
        // setup plane VAO
            glGenVertexArrays(1, &mQuadVAO);
            glGenBuffers(1, &mQuadVBO);
            glBindVertexArray(mQuadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, mQuadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
            glBindVertexArray(0);
        }

        virtual void deleteResources()
        {
            YYLog::D("DeferredShading deleteResources");
            glDeleteRenderbuffers(1, &gDepthRBO);
            glDeleteTextures(1, &gPoitionTex);
            glDeleteTextures(1, &gNormalTex);
            glDeleteTextures(1, &gAlbedoSpecTex);
            glDeleteFramebuffers(1, &gBufferFBO);
            mLightPositions.clear();
            mLightColors.clear();
            glDeleteBuffers(1, &mQuadVBO);
            glDeleteVertexArrays(1, &mQuadVAO);
        }
    };
}

std::shared_ptr<YYGLModule3D> yyInitDeferredShading(void* assetMgr, const char* modelPath, bool flipV)
{
    std::shared_ptr<YYGLModule3D::YYGLModule3DData> data = std::make_shared<YYGLModule3D::YYGLModule3DData>();
    data->mAssetMgr = assetMgr;
    data->modelsLoadPath.push_back(std::string(modelPath));
    data->modelsLoadPath.push_back(CubeModelPath);
    data->mLoadTexFlipV = flipV;
    YYGLModule3D::YYGLModule3DData::ProgramShaderPathPackage gBufferPackage = {
        GBufferProgramName,
        "shaders/3D/gBuffer.vs",
        "shaders/3D/gBuffer.fs",
        ""
    };
    data->mProgramPathPackages.push_back(gBufferPackage);
    YYGLModule3D::YYGLModule3DData::ProgramShaderPathPackage lightPackage = {
        DeferredShadingProgramName,
        "shaders/3D/deferred_shading.vs",
        "shaders/3D/deferred_shading.fs",
        ""
    };
    data->mProgramPathPackages.push_back(lightPackage);
    YYGLModule3D::YYGLModule3DData::ProgramShaderPathPackage boxPackage = {
        CubeBoxProgramName,
        "shaders/3D/box.vs",
        "shaders/3D/box.fs",
        ""
    };
    data->mProgramPathPackages.push_back(boxPackage);
    data->mEnableDepthTest = true;
    data->mEnableClearColor = true;
    data->mEnableCullFace = true;
    std::shared_ptr<YYGLModule3D> modulePtr = std::make_shared<DeferredShading>(data);
    modulePtr->grInitModule();

    return modulePtr;
}


unsigned int yyProcessDeferredShading(const std::shared_ptr<YYGLModule3D> modulePtr,
                                    unsigned int targetColorTexID,
                                    int targetWidth,
                                    int targetHeight,
                                    glm::mat4 projectionMat,
                                    glm::mat4 viewMat,
                                    std::vector<glm::mat4> modelMats,
                                    glm::vec3 cameraPos)
{
    std::shared_ptr<YYGLModule3D::YYGLModule3DData> data = modulePtr->getYYGLModuleData();
    assert(data != nullptr);
    data->mRegenerateResources = ((targetWidth != data->mTargetWidth) || (targetHeight != data->mTargetHeight));
    data->mTargetWidth = targetWidth;
    data->mTargetHeight = targetHeight;
    data->mColorTargetTexID = targetColorTexID;
    data->mProjectionMat4 = projectionMat;
    data->mModelMat4s = modelMats;
    data->mViewMat4 = viewMat;
    data->mCameraPos = cameraPos;
    return modulePtr->grProcessModule();
}