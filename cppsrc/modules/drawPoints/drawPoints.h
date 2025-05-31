// Copyright 2022, YYAL.
// SPDX-License-Identifier: BSL-1.0

/*!
 * @file
 * @brief  This is qcom foveation module which using OpenGL ES32
 * @author Yaolong Yu <yaolong-yu@ylab.ac.cn>
 */

#ifndef GRDRAWPOINTS_H
#define GRDRAWPOINTS_H

#ifdef __cplusplus
    extern "C" {
#endif

/*!
 * @function
 * @brief draw points on source texture, will return a result new texture
 *        Caution: this process will enable cull face, front face is counter clockwise,
 *                 depth and stencil test will be disabled
 *                 neither glFinish nor glFlush will be called in the process.
 *                 Caller can do either after grProcessXXX call by demands
 *        Please set back to your GL states when process finish
 * @input
 *        texId: texture ID will draw points on this texture
 *        reqWidth: required return texture width
 *        reqHeight required return texture height
 *        pointsData: points NDC coordinate data
 *        dataSize: data size of points data
 * @return
 *        N/A
 */
void grInitDrawPoints(int targetWidth,
                      int targetHeight);

/*!
 * @function
 * @brief Apply drawing points process on source texture
 *
 * @input
 *        N/A
 * @return
 *        N/A
 */
void grProcessDrawPoints(unsigned int inPlaceTexID,
                         float* pointsData,
                         int dataSize,
                         float colorR, float colorG, float colorB);

/*!
 * @function
 * @brief finish AADT module, deallocate resources,
 * @input N/A
 * @return N/A
 */

void grProcessDrawPointsOnScreen(float* pointsData,
                                 int dataSize,
                                 float colorR,
                                 float colorG,
                                 float colorB,
                                 int lowLeftX,
                                 int lowLeftY,
                                 int screenWidth,
                                 int screenHeight);

void grReleaseDrawPoints();

#ifdef __cplusplus
}
#endif

#endif
