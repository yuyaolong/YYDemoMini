//
// Created by yuyao on 2024/2/18.
//

#pragma once
#include "YYGLModule.hpp"

std::shared_ptr<YYGLModule> yyInitGaussianSplatRender(void* assetMgr, bool use2DArraySrcTex);


void yyProcessGaussianSplatRender(const std::shared_ptr<YYGLModule> modulePtr,
                                 unsigned int srcTexID,
                                 unsigned int targetColorTexID,
                                 unsigned int targetDepthTexID,
                                 int targetWidth,
                                 int targetHeight,
                                 bool enableDepth);

void yyGaussianSplatRenderCameraZoom(const std::shared_ptr<YYGLModule>,
                                     float factor);



