// Copyright 2022, YYAL.
// SPDX-License-Identifier: BSL-1.0

/*!
 * @file
 * @brief  This is qcom foveation module which using OpenGL ES32
 * @author Yaolong Yu <yaolong-yu@ylab.ac.cn>
 */

#ifndef GRSHOWONSCREENMULTIVIEW_H
#define GRSHOWONSCREENMULTIVIEW_H

#ifdef __cplusplus
    extern "C" {
#endif


void grInitShowOnScreenMultiview(bool useSubSampleLayout);


void grProcessShowOnScreenMultiview(unsigned int showTex2DArrayID,
                                   unsigned int showTex2DArrayDepthID,
                                   int lowLeftX,
                                   int lowLeftY,
                                   int screenWidth,
                                   int screenHeight,
                                   bool showDepth);


void grReleaseShowOnScreenMultiview();

#ifdef __cplusplus
}
#endif

#endif
