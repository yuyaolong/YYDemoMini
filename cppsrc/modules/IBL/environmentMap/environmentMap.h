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


void grInitEnvironmentMap(void* assetMgr);


unsigned int grProcessEnvironmentMap(unsigned int showTexID, // cube map tex
                             int screenWidth,
                             int screenHeight,
                             unsigned int targetColorTexID,
                             unsigned int targetDepthTexID,
                             unsigned int targetDepthRbo,
                             void* viewMat4,
                             void* projectionMat4);


void grReleaseEnvironmentMap();

#ifdef __cplusplus
}
#endif

