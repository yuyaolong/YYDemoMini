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


void grInitIrradianceMap(void* assetMgr);

unsigned int grProcessIrradianceMap(unsigned int srcColorTexID, //cubeMap
                             unsigned int targetColorTexID, //cubeMap
                             unsigned int targetDepthRbo,
                             int targetWidth,
                             int targetHeight);


void grReleaseIrradianceMap();

#ifdef __cplusplus
}
#endif

