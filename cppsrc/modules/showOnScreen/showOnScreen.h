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


void grInitShowOnScreen(void* assetMgr);


void grProcessShowOnScreen(unsigned int showTexID,
                           int lowLeftX,
                           int lowLeftY,
                           int screenWidth,
                           int screenHeight);


void grReleaseShowOnScreen();

#ifdef __cplusplus
}
#endif
