//
// Created by yuyao on 2024/2/18.
//

#pragma once
#include "YYGLModule3D.hpp"

std::shared_ptr<YYGLModule3D> yyInitDeferredShading(void* assetMgr, const char* modelPath, bool flipV);


unsigned int yyProcessDeferredShading(const std::shared_ptr<YYGLModule3D> modulePtr,
                                unsigned int targetColorTexID,
                                int targetWidth,
                                int targetHeight,
                                glm::mat4 projectionMat,
                                glm::mat4 viewMat,
                                std::vector<glm::mat4> modelMats,
                                glm::vec3 cameraPos);



