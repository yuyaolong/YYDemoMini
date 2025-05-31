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


void grInitDrawGrid(int targetWidth,
                    int targetHeight,
                    int gridHorizonNum,
                    int gridVerticalNum);


void grProcessDrawGrid(unsigned int targetTexID,
                       float lineWidth);


void grReleaseDrawGrid();

#ifdef __cplusplus
}
#endif