// Copyright 2022, YYAL.
// SPDX-License-Identifier: BSL-1.0

/*!
 * @file
 * @brief  This is qcom foveation module which using OpenGL ES32
 * @author Yaolong Yu <yaolong-yu@ylab.ac.cn>
 */

#pragma once

#ifdef __cplusplus
    extern "C" {
#endif


void grInitPbrRender(void* assetMgr,
                     const float* modelDataPtr,
                     int modelDataLen,
                     const unsigned int* elementDataPtr,
                     int elementDataLen,
                     int drawType,
                     const int* attributesSizeArray,
                     int attributesSizeArrayLen,
                     int verticesNum,
                     const char* materialFolder,
                     const float* SH9_vec4s); // RGB + useless


unsigned int grProcessPbrRender(unsigned int irradianceMapTexID, //cubeMap Tex
                                unsigned int specularPrefilterMapTexID, //cubeMap Tex
                                unsigned int specularBRDFLutTexID, //2D Tex
                                int targetWidth,
                                int targetHeight,
                                unsigned int targetColorTexID,
                                unsigned int targetDepthTexID,
                                unsigned int targetDepthRbo,
                                void* modelMat4,
                                void* viewMat4,
                                void* projectionMat4,
                                void* viewPositionVec3,
                                bool clearColor);


void grReleasePbrRender();

#ifdef __cplusplus
}
#endif

