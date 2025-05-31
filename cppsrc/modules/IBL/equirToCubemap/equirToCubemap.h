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


void grInitEquirToCubemap(void* assetMgr);

// targetColorTexID Need to be cube map tex type
unsigned int grProcessEquirToCubemap(unsigned int srcColorTexID, // tex2D
                             unsigned int targetColorTexID, //texCubeMap
                             unsigned int targetDepthRbo,
                             int targetWidth,
                             int targetHeight);


void grReleaseEquirToCubemap();

#ifdef __cplusplus
}
#endif

