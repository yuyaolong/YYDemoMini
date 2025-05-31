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

#define SH_COUNT 9

typedef struct
{
    double coeffs[SH_COUNT];
} ShChannel;

void grProcessSH9Extract(unsigned int irradianceTexID, // cube map tex
                                 int texWidth,
                                 int texHeight,
                                 unsigned int components_per_pixel,
                                 ShChannel* out_channels);

#ifdef __cplusplus
}
#endif

