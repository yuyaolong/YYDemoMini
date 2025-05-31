//
// Created by yuyao on 2024/2/18.
//

#pragma once
#include "YYGLModule3D.hpp"

std::shared_ptr<YYGLModule3D> yyInitModelRender(void* assetMgr, const char* modelPath, bool flipV=false);


void yyProcessModelRender(const std::shared_ptr<YYGLModule3D> modulePtr,
                         unsigned int targetColorTexID,
                         unsigned int targetDepthTexID,
                         int targetWidth,
                         int targetHeight,
                         glm::mat4 projectionMat,
                         glm::mat4 viewMat,
                         std::vector<glm::mat4> modelMats);



