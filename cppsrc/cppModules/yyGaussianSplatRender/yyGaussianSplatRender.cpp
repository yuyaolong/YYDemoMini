//
// Created by yuyao on 2024/2/18.
//

#include "yyGaussianSplatRender.h"
//#include "YYGLModelData.h"
#include "yyPlyParser.h"
#include "YYProgram.h"
#include "YYBufferObject.h"
#include "radix_sort.hpp"
#include <sstream>

namespace {
    class GaussianSplatRender : public YYGLModule {
    public:
        struct GaussianSplatRenderModuleData : YYGLModuleData {

        };
        explicit GaussianSplatRender(std::shared_ptr<GaussianSplatRenderModuleData> data) : YYGLModule(data) { YYLog::D("GaussianSplatRender create"); }
        ~GaussianSplatRender() { YYLog::D("GaussianSplatRender destroy"); };

        void grInitModule() override {
            // parse gaussian data
            parsePlyData();

            // Load pre-sort compute program
            YY_DEMO_ASSERT(mModuleData->mAssetMgr != nullptr)
            mPreSortProg = std::make_shared<YYProgram>(mModuleData->mAssetMgr);
            if (!mPreSortProg->LoadCompute("shaders/gaussianSplat/presortCompute.glsl")) {
                YYLog::E("Error loading point pre-sort compute shader!");
            }

            mSplatProg = std::make_shared<YYProgram>(mModuleData->mAssetMgr);

            if (mIsFramebufferSRGBEnabled || mUseFullSH)
            {
                std::string defines = "";
                if (mIsFramebufferSRGBEnabled)
                {
                    defines += "#define FRAMEBUFFER_SRGB\n";
                }
                if (mUseFullSH)
                {
                    defines += "#define FULL_SH\n";
                }
                mSplatProg->AddMacro("DEFINES", defines);
            }

            if (!mSplatProg->LoadVertGeomFrag("shaders/gaussianSplat/splat_vert.glsl", "shaders/gaussianSplat/splat_geom.glsl", "shaders/gaussianSplat/splat_frag.glsl"))
            {
                YYLog::E("Error loading splat shaders!");
            }


            prepareSortBufferObjects();

            radixSorter = std::make_shared<rgc::radix_sort::sorter>(mGaussianVec.size());

            prepareSplatVAO();
        }

        virtual unsigned int grProcessModule() override {
            // prepare mats and vecs
            CHECK_GL_ERROR
            mViewPort = {0.0f, 0.0f, mModuleData->mTargetWidth, mModuleData->mTargetHeight};
            mProjectionMat = glm::perspective(glm::radians(90.0f), (float)mModuleData->mTargetWidth / (float)mModuleData->mTargetHeight, mNearFar.x, mNearFar.y);
            mViewMat = glm::lookAt(mEyePos, mCenter, glm::vec3(0.0, 1.0, 0.0));

            doPresort();

            mAtomicCounterBuffer->Read(atomicCounterVec);
            assert(atomicCounterVec[0] <= mGaussianVec.size());
            // mValBuffer is indeices buffer
            radixSorter->sort(mKeyBuffer->GetObj(), mValBuffer->GetObj(), atomicCounterVec[0]);

            //key: value buffer will copy to element buffer
            glBindBuffer(GL_COPY_READ_BUFFER, mValBuffer->GetObj());
            glBindBuffer(GL_COPY_WRITE_BUFFER, mSplatVao->GetElementBuffer()->GetObj());
            glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, atomicCounterVec[0] * sizeof(uint32_t));

            printSortBuffers(true);


            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glEnable(GL_BLEND);
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_DEPTH_TEST);

            glViewport(mViewPort.x, mViewPort.y, mViewPort.z, mViewPort.w);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            CHECK_GL_ERROR

            mSplatProg->Bind();
            mSplatProg->SetUniform("viewMat", mViewMat);
            mSplatProg->SetUniform("projMat", mProjectionMat);
            mSplatProg->SetUniform("viewport", mViewPort);
            mSplatProg->SetUniform("projParams", glm::vec4(0.0f, mNearFar.x, mNearFar.y, 0.0f));
            mSplatProg->SetUniform("eye", mEyePos);

            // test later
            mSplatVao->Bind();
            glDrawElements(GL_POINTS, atomicCounterVec[0], GL_UNSIGNED_INT, nullptr);
            mSplatVao->Unbind();

            return 0;
        }

        void cameraZoom(float scaleFactor) {
            mEyePos = (mEyePos - mCenter) / scaleFactor + mCenter;
        }

        GaussianSplatRender(const GaussianSplatRender&) = delete;
        GaussianSplatRender& operator=(const GaussianSplatRender&) = delete;

    private:
        struct
        {
            YYPlyParser::Property x, y, z;
            YYPlyParser::Property f_dc[3];
            YYPlyParser::Property f_rest[45];
            YYPlyParser::Property opacity;
            YYPlyParser::Property scale[3];
            YYPlyParser::Property rot[4];
        } props;

        struct Gaussian
        {
            float position[3];  // in world space
            float normal[3];  // unused
            float f_dc[3];  // first order spherical harmonics coeff (sRGB color space)
            float f_rest[45];  // more spherical harminics coeff
            float opacity;  // alpha = 1 / (1 + exp(-opacity));
            float scale[3];
            float rot[4];  // local rotation of guassian (real, i, j, k)

            // convert from (scale, rot) into the gaussian covariance matrix in world space
            // See 3d Gaussian Splat paper for more info
            glm::mat3 ComputeCovMat() const
            {
                glm::quat q(rot[0], rot[1], rot[2], rot[3]);
                glm::mat3 R(glm::normalize(q));
                glm::mat3 S(glm::vec3(expf(scale[0]), 0.0f, 0.0f),
                glm::vec3(0.0f, expf(scale[1]), 0.0f),
                        glm::vec3(0.0f, 0.0f, expf(scale[2])));
                return R * S * glm::transpose(S) * glm::transpose(R);
            }
        };



        void parsePlyData () {
            // read ply data
            YYPlyParser plyParser;

            // use asset manager
            // open shader file
            AAsset* plyAssetFile = AAssetManager_open(static_cast<AAssetManager*>(mModuleData->mAssetMgr), "plyFiles/can.ply", AASSET_MODE_BUFFER);
            YY_DEMO_ASSERT(plyAssetFile != nullptr);
            size_t shaderBufferLen = AAsset_getLength(plyAssetFile);
            YY_DEMO_ASSERT(shaderBufferLen != 0);
            std::istringstream plyFile(std::string( static_cast<const char*>(AAsset_getBuffer(plyAssetFile)), shaderBufferLen));
            //  additVonal terminating null-character ('\0') at the end automatically when c_str() return

            if (!plyParser.Parse(plyFile))
            {
                YYLog::E("Error parsing ply file \n");
            }

            if (!plyParser.GetProperty("x", props.x) ||
                !plyParser.GetProperty("y", props.y) ||
                !plyParser.GetProperty("z", props.z))
            {
                YYLog::E("Error parsing ply file, missing position property");
            }

            for (int i = 0; i < 3; i++)
            {
                if (!plyParser.GetProperty("f_dc_" + std::to_string(i), props.f_dc[i]))
                {
                    YYLog::E("Error parsing ply file, missing f_dc property");
                }
            }

            for (int i = 0; i < 45; i++)
            {
                if (!plyParser.GetProperty("f_rest_" + std::to_string(i), props.f_rest[i]))
                {
                    YYLog::E("Error parsing ply file, missing f_rest property");
                }
            }

            if (!plyParser.GetProperty("opacity", props.opacity))
            {
                YYLog::E("Error parsing ply file, missing opacity property");
            }

            for (int i = 0; i < 3; i++)
            {
                if (!plyParser.GetProperty("scale_" + std::to_string(i), props.scale[i]))
                {
                    YYLog::E("Error parsing ply file, missing scale property");
                }
            }

            for (int i = 0; i < 4; i++)
            {
                if (!plyParser.GetProperty("rot_" + std::to_string(i), props.rot[i]))
                {
                    YYLog::E("Error parsing ply file, missing rot property");
                }
            }

            mGaussianVec.resize(plyParser.GetVertexCount());

            int i = 0;
            plyParser.ForEachVertex([this, &i](const uint8_t* data, size_t size)
              {
                  mGaussianVec[i].position[0] = props.x.Get<float>(data);
                  mGaussianVec[i].position[1] = props.y.Get<float>(data);
                  mGaussianVec[i].position[2] = props.z.Get<float>(data);
                  for (int j = 0; j < 3; j++)
                  {
                      mGaussianVec[i].f_dc[j] = props.f_dc[j].Get<float>(data);
                  }
                  for (int j = 0; j < 45; j++)
                  {
                      mGaussianVec[i].f_rest[j] = props.f_rest[j].Get<float>(data);
                  }
                  mGaussianVec[i].opacity = props.opacity.Get<float>(data);
                  for (int j = 0; j < 3; j++)
                  {
                      mGaussianVec[i].scale[j] = props.scale[j].Get<float>(data);
                  }
                  for (int j = 0; j < 4; j++)
                  {
                      mGaussianVec[i].rot[j] = props.rot[j].Get<float>(data);
                  }
                  i++;
              });
            YYLog::I("parse ply finish");
        }


        void printSortBuffers(bool eleBuffer = false) {
            size_t  numPoints = mGaussianVec.size();

            if (eleBuffer == true)
            {
                mSplatVao->GetElementBuffer()->Bind();
                uint32_t* ck0 = (uint32_t*)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, numPoints * sizeof(uint32_t), GL_MAP_READ_BIT);
                CHECK_GL_ERROR
                for (int i = 0; i < numPoints; ++i) {
                    YYLog::D("element %u", *(ck0+i));
                }
                glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

                mSplatVao->GetElementBuffer()->Unbind();
            }

            mValBuffer->Bind();
            uint32_t* ck1 = (uint32_t*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, numPoints * sizeof(uint32_t), GL_MAP_READ_BIT);
            for (int i = 0; i < numPoints; ++i) {
                YYLog::D("indices %u", *(ck1+i));
            }
            glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
            mValBuffer->Unbind();

            mKeyBuffer->Bind();
            uint32_t* ck2 = (uint32_t*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, numPoints * sizeof(uint32_t), GL_MAP_READ_BIT);
            for (int i = 0; i < numPoints; ++i) {
                YYLog::D("depth %u", *(ck2+i));
            }
            glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
            mKeyBuffer->Unbind();

            mPosBuffer->Bind();
            float* ck3 = (float*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, numPoints * sizeof(uint32_t), GL_MAP_READ_BIT);
            for (int i = 0; i < numPoints; ++i) {
                YYLog::D("%f, %f, %f, %f", *(ck3 + i * 4 + 0),
                                               *(ck3 + i * 4 + 1),
                                               *(ck3 + i * 4 + 2),
                                               *(ck3 + i * 4 + 3));
            }
            glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
            mPosBuffer->Unbind();
        }

        void prepareSortBufferObjects()
        {
            uint32_t i = 0;
            size_t gsNum = mGaussianVec.size();
            posVec.reserve(gsNum);
            depthVec.resize(gsNum);
            indexVec.reserve(gsNum);
            for (const Gaussian& g : mGaussianVec)
            {
                float alpha = 1.0f / (1.0f + expf(-g.opacity));
                posVec.emplace_back(glm::vec4(g.position[0], g.position[1], g.position[2], alpha));
                indexVec.push_back(i);
                i++;
            }

            atomicCounterVec.resize(1, 0);
            mAtomicCounterBuffer = std::make_shared<BufferObject>(GL_ATOMIC_COUNTER_BUFFER, atomicCounterVec, GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT);
            mKeyBuffer = std::make_shared<BufferObject>(GL_SHADER_STORAGE_BUFFER, depthVec, GL_MAP_READ_BIT);
            mValBuffer = std::make_shared<BufferObject>(GL_SHADER_STORAGE_BUFFER, indexVec, GL_MAP_READ_BIT);
            mPosBuffer = std::make_shared<BufferObject>(GL_SHADER_STORAGE_BUFFER, posVec, GL_MAP_READ_BIT);

            printSortBuffers();

        }

        // prepareSortBufferObjects need to happen firstly
        void prepareSplatVAO()
        {
            mSplatVao = std::make_shared<VertexArrayObject>();
            size_t numGs = mGaussianVec.size();

            // sh coeff
            std::vector<glm::vec4> r_sh0Vec, g_sh0Vec, b_sh0Vec;
            std::vector<glm::vec4> r_sh1Vec, r_sh2Vec, r_sh3Vec, r_sh4Vec;
            std::vector<glm::vec4> g_sh1Vec, g_sh2Vec, g_sh3Vec, g_sh4Vec;
            std::vector<glm::vec4> b_sh1Vec, b_sh2Vec, b_sh3Vec, b_sh4Vec;

            // 3x3 cov matrix
            std::vector<glm::vec3> cov3_col0Vec, cov3_col1Vec, cov3_col2Vec;

            r_sh0Vec.reserve(numGs);
            g_sh0Vec.reserve(numGs);
            b_sh0Vec.reserve(numGs);

            if (mUseFullSH)
            {
                r_sh1Vec.reserve(numGs);
                r_sh2Vec.reserve(numGs);
                r_sh3Vec.reserve(numGs);
                g_sh1Vec.reserve(numGs);
                g_sh2Vec.reserve(numGs);
                g_sh3Vec.reserve(numGs);
                b_sh1Vec.reserve(numGs);
                b_sh2Vec.reserve(numGs);
                b_sh3Vec.reserve(numGs);
            }

            cov3_col0Vec.reserve(numGs);
            cov3_col1Vec.reserve(numGs);
            cov3_col2Vec.reserve(numGs);

            for (auto&& g : mGaussianVec)
            {
                r_sh0Vec.emplace_back(glm::vec4(g.f_dc[0], g.f_rest[0], g.f_rest[1], g.f_rest[2]));
                g_sh0Vec.emplace_back(glm::vec4(g.f_dc[1], g.f_rest[15], g.f_rest[16], g.f_rest[17]));
                b_sh0Vec.emplace_back(glm::vec4(g.f_dc[2], g.f_rest[30], g.f_rest[31], g.f_rest[32]));

                if (mUseFullSH)
                {
                    r_sh1Vec.emplace_back(glm::vec4(g.f_rest[3], g.f_rest[4], g.f_rest[5], g.f_rest[6]));
                    r_sh2Vec.emplace_back(glm::vec4(g.f_rest[7], g.f_rest[8], g.f_rest[9], g.f_rest[10]));
                    r_sh3Vec.emplace_back(glm::vec4(g.f_rest[11], g.f_rest[12], g.f_rest[13], g.f_rest[14]));
                    g_sh1Vec.emplace_back(glm::vec4(g.f_rest[18], g.f_rest[19], g.f_rest[20], g.f_rest[21]));
                    g_sh2Vec.emplace_back(glm::vec4(g.f_rest[22], g.f_rest[23], g.f_rest[24], g.f_rest[25]));
                    g_sh3Vec.emplace_back(glm::vec4(g.f_rest[26], g.f_rest[27], g.f_rest[28], g.f_rest[29]));
                    b_sh1Vec.emplace_back(glm::vec4(g.f_rest[33], g.f_rest[34], g.f_rest[35], g.f_rest[36]));
                    b_sh2Vec.emplace_back(glm::vec4(g.f_rest[37], g.f_rest[38], g.f_rest[39], g.f_rest[40]));
                    b_sh3Vec.emplace_back(glm::vec4(g.f_rest[41], g.f_rest[42], g.f_rest[43], g.f_rest[44]));
                }

                glm::mat3 V = g.ComputeCovMat();
                cov3_col0Vec.push_back(V[0]);
                cov3_col1Vec.push_back(V[1]);
                cov3_col2Vec.push_back(V[2]);
            }

            auto indexBuffer = std::make_shared<BufferObject>(GL_ELEMENT_ARRAY_BUFFER, indexVec, GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT);
            auto positionBuffer = std::make_shared<BufferObject>(GL_ARRAY_BUFFER, posVec);

            auto r_sh0Buffer = std::make_shared<BufferObject>(GL_ARRAY_BUFFER, r_sh0Vec);
            auto g_sh0Buffer = std::make_shared<BufferObject>(GL_ARRAY_BUFFER, g_sh0Vec);
            auto b_sh0Buffer = std::make_shared<BufferObject>(GL_ARRAY_BUFFER, b_sh0Vec);

            auto r_sh1Buffer = std::make_shared<BufferObject>(GL_ARRAY_BUFFER, r_sh1Vec);
            auto r_sh2Buffer = std::make_shared<BufferObject>(GL_ARRAY_BUFFER, r_sh2Vec);
            auto r_sh3Buffer = std::make_shared<BufferObject>(GL_ARRAY_BUFFER, r_sh3Vec);
            auto g_sh1Buffer = std::make_shared<BufferObject>(GL_ARRAY_BUFFER, g_sh1Vec);
            auto g_sh2Buffer = std::make_shared<BufferObject>(GL_ARRAY_BUFFER, g_sh2Vec);
            auto g_sh3Buffer = std::make_shared<BufferObject>(GL_ARRAY_BUFFER, g_sh3Vec);
            auto b_sh1Buffer = std::make_shared<BufferObject>(GL_ARRAY_BUFFER, b_sh1Vec);
            auto b_sh2Buffer = std::make_shared<BufferObject>(GL_ARRAY_BUFFER, b_sh2Vec);
            auto b_sh3Buffer = std::make_shared<BufferObject>(GL_ARRAY_BUFFER, b_sh3Vec);

            auto cov3_col0Buffer = std::make_shared<BufferObject>(GL_ARRAY_BUFFER, cov3_col0Vec);
            auto cov3_col1Buffer = std::make_shared<BufferObject>(GL_ARRAY_BUFFER, cov3_col1Vec);
            auto cov3_col2Buffer = std::make_shared<BufferObject>(GL_ARRAY_BUFFER, cov3_col2Vec);
            CHECK_GL_ERROR
            // setup vertex array object with buffers
            mSplatVao->SetAttribBuffer(mSplatProg->GetAttribLoc("position"), positionBuffer);

            mSplatVao->SetAttribBuffer(mSplatProg->GetAttribLoc("r_sh0"), r_sh0Buffer);
            mSplatVao->SetAttribBuffer(mSplatProg->GetAttribLoc("g_sh0"), g_sh0Buffer);
            mSplatVao->SetAttribBuffer(mSplatProg->GetAttribLoc("b_sh0"), b_sh0Buffer);

            if (mUseFullSH)
            {
                mSplatVao->SetAttribBuffer(mSplatProg->GetAttribLoc("r_sh1"), r_sh1Buffer);
                mSplatVao->SetAttribBuffer(mSplatProg->GetAttribLoc("r_sh2"), r_sh2Buffer);
                mSplatVao->SetAttribBuffer(mSplatProg->GetAttribLoc("r_sh3"), r_sh3Buffer);
                mSplatVao->SetAttribBuffer(mSplatProg->GetAttribLoc("g_sh1"), g_sh1Buffer);
                mSplatVao->SetAttribBuffer(mSplatProg->GetAttribLoc("g_sh2"), g_sh2Buffer);
                mSplatVao->SetAttribBuffer(mSplatProg->GetAttribLoc("g_sh3"), g_sh3Buffer);
                mSplatVao->SetAttribBuffer(mSplatProg->GetAttribLoc("b_sh1"), b_sh1Buffer);
                mSplatVao->SetAttribBuffer(mSplatProg->GetAttribLoc("b_sh2"), b_sh2Buffer);
                mSplatVao->SetAttribBuffer(mSplatProg->GetAttribLoc("b_sh3"), b_sh3Buffer);
            }

            mSplatVao->SetAttribBuffer(mSplatProg->GetAttribLoc("cov3_col0"), cov3_col0Buffer);
            mSplatVao->SetAttribBuffer(mSplatProg->GetAttribLoc("cov3_col1"), cov3_col1Buffer);
            mSplatVao->SetAttribBuffer(mSplatProg->GetAttribLoc("cov3_col2"), cov3_col2Buffer);
            mSplatVao->SetElementBuffer(indexBuffer);
            CHECK_GL_ERROR
        }

        void doPresort()
        {
            glm::mat4 mvpMat = mProjectionMat * mViewMat;
            CHECK_GL_ERROR
            mPreSortProg->Bind();
            mPreSortProg->SetUniform("modelViewProj", mvpMat);
            mPreSortProg->SetUniform("nearFar", mNearFar);
            mPreSortProg->SetUniform("keyMax", std::numeric_limits<uint32_t>::max());
            CHECK_GL_ERROR
            // reset counter back to zero
            atomicCounterVec[0] = 0;
            mAtomicCounterBuffer->Update(atomicCounterVec);
            CHECK_GL_ERROR

            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mPosBuffer->GetObj());  // readonly
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mKeyBuffer->GetObj());  // writeonly
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mValBuffer->GetObj());  // writeonly
            glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 4, mAtomicCounterBuffer->GetObj());
            CHECK_GL_ERROR

            const int LOCAL_SIZE = 256;
            const size_t numPoints = posVec.size();
            glDispatchCompute(((GLuint)numPoints + (LOCAL_SIZE - 1)) / LOCAL_SIZE, 1, 1); // Assuming LOCAL_SIZE threads per group
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);

            CHECK_GL_ERROR

            printSortBuffers();
        }

        // use YYSettings later
        bool mIsFramebufferSRGBEnabled = false;
        bool mUseFullSH = false;

        // radix sorter
        std::shared_ptr<rgc::radix_sort::sorter> radixSorter;

        std::vector<glm::vec4> posVec;
        std::vector<uint32_t> indexVec;
        std::vector<uint32_t> depthVec;

        std::vector<uint32_t> atomicCounterVec; // key: this will be the draw sort and draw splat count

        std::shared_ptr<BufferObject> mAtomicCounterBuffer;
        std::shared_ptr<BufferObject> mPosBuffer;
        std::shared_ptr<BufferObject> mKeyBuffer;
        std::shared_ptr<BufferObject> mValBuffer;

        std::vector<Gaussian> mGaussianVec;
        std::shared_ptr<YYProgram> mPreSortProg;
        std::shared_ptr<YYProgram> mSplatProg;

        std::shared_ptr<VertexArrayObject> mSplatVao;

        glm::vec4 mViewPort;
        glm::vec2 mNearFar = {0.01f, 1000.0f};
        glm::mat4 mViewMat;
        glm::mat4 mProjectionMat;
        glm::vec3 mEyePos = {0.0f, 1.0f, 2.0f};
        glm::vec3 mCenter = {0.0f, 0.0f, 0.0f};

    };
}

std::shared_ptr<YYGLModule> yyInitGaussianSplatRender(void* assetMgr, bool use2DArraySrcTex)
{
    std::shared_ptr<GaussianSplatRender::GaussianSplatRenderModuleData> data = std::make_shared<GaussianSplatRender::GaussianSplatRenderModuleData>();
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

    std::shared_ptr<YYGLModule> modulePtr = std::make_shared<GaussianSplatRender>(data);
    modulePtr->grInitModule();

    return modulePtr;
}

void yyProcessGaussianSplatRender(const std::shared_ptr<YYGLModule> modulePtr,
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

void yyGaussianSplatRenderCameraZoom(const std::shared_ptr<YYGLModule> modulePtr,
                                     float factor)
{
    std::dynamic_pointer_cast<GaussianSplatRender>(modulePtr)->cameraZoom(factor);
}