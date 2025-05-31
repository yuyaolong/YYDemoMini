// Copyright 2022, YYAL.
// SPDX-License-Identifier: BSL-1.0
#ifndef FOVEATIONBYVRS_H
#define FOVEATIONBYVRS_H

#ifdef __cplusplus
    extern "C" {
#endif

unsigned int grInitFoveationByVRS(unsigned int srcTexID,
                                 int targetWidth,
                                 int targetHeight);

void grProcessFoveationByVRS(void* fovPara, int showTexture);

void grReleaseFoveationByVRS();

#ifdef __cplusplus
}
#endif

#endif
