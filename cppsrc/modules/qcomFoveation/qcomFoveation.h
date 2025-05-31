// Copyright 2022, YYAL.
// SPDX-License-Identifier: BSL-1.0

/*!
 * @file
 * @brief  This is qcom foveation module which using OpenGL ES32
 * @author Yaolong Yu <yaolong-yu@ylab.ac.cn>
 */

#ifndef GRQCOMFOVEATION_H
#define GRQCOMFOVEATION_H

#ifdef __cplusplus
    extern "C" {
#endif

/*!
 * @function
 * @brief init qcom foveation module, allocate resources for the first time
 *        Caution: this process will enable cull face, front face is counter clockwise,
 *                 depth and stencil test will be disabled
 *                 neither glFinish nor glFlush will be called in the process.
 *                 Caller can do either after grProcessXXX call by demands
 *        Please set back to your GL states when process finish
 * @input
 *        srcTexId: source texture ID
 *        srcWidth: source texture width
 *        srcHeight source texture height
 * @output
 *        outWidth: pointer for return texture width
 *        outHeight: pointer for return texture height
 * @return
 *        targetTexId: target texture Id
 */
unsigned int grInitFoveationQcom(unsigned int srcTexID,
                                 int targetWidth,
                                 int targetHeight,
                                 bool useSubSampleLayout);

/*!
 * @function
 * @brief Apply QCOM foveation process on source texture
 *        algorithm will always down sample srcWidth and srcHeight to half
 * @input
 *        N/A
 * @return
 *        N/A
 */
unsigned int grProcessFoveationQcom(void* pFovPara,
                                    int focalNum,
                                    int showTexture);

/*!
 * @function
 * @brief finish AADT module, deallocate resources,
 * @input N/A
 * @return N/A
 */
void grReleaseFoveationQcom();

#ifdef __cplusplus
}
#endif

#endif
