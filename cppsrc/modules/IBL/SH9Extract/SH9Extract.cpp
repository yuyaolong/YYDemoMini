//
// Copyright (c) 2022-present, YYAL.  All rights reserved.
// Created by Yaolong Yu on 2022/8/11.
//

#include "SH9Extract.h"
#include "miscUtils.h"

namespace {
    int g_texWidth = 0;
    int g_texHeight = 0;
    std::vector<const char*> g_faceName = {
            "PX",
            "NX",
            "PY",
            "NY",
            "PZ",
            "NZ"
    };

    static inline double sh_eval_9(int i, double x, double y, double z)
    {
        const float sh0 = 0.28209479f;

        // 1,-1 y  1,0 z  1,1 x
        const float sh1 = 0.48860251f;

        // 2,-2 xy   
        const float sh2_2 = 1.09254843f;

        // 2,-1 yz
        const float sh2_1 = 1.09254843f;

        // 2,0 -x^2 - y^2 + 2z^2
        const float sh20 = 0.31539157f;

        // 2, 1 zx
        const float sh21 = 1.09254843f;

        // 2, 2 x^2 - y^2
        const float sh22 = 0.54627421f;
        switch (i)
        {
            case 0:
                return sh0;
            case 1:
                return sh1 * y;
            case 2:
                return sh1 * z;
            case 3:
                return sh1 * x;
            case 4:
                return sh2_2 * x * y;
            case 5:
                return sh2_1 * y * z;
            case 6:
                return sh20 * (-x * x - y * y + 2 * z * z);
            case 7:
                return sh21 * z * x;
            case 8:
                return sh22 * (x * x - y * y);
            default:
                assert(0);
                return 0;
        }
    }


    static inline void normalize(double* dir, int n)
    {
        double length_sqr = 0.0f;
        for (int i = 0; i < n; ++i)
            length_sqr += dir[i] * dir[i];

        double scale = 1.0 / sqrt(length_sqr);
        for (int i = 0; i < n; ++i)
            dir[i] *= scale;
    }

    static inline void uv_to_cube(double u, double v, int face, double* out_dir)
    {
        switch (face)
        {
            case 0: // PX
                out_dir[0] = 1.0f;
                out_dir[1] = -v;
                out_dir[2] = -u;
                break;
            case 1: // NX
                out_dir[0] = -1.0f;
                out_dir[1] = -v;
                out_dir[2] = u;
                break;
            case 2: // PY
                out_dir[0] = u;
                out_dir[1] = 1.0f;
                out_dir[2] = v;
                break;
            case 3: // NY
                out_dir[0] = u;
                out_dir[1] = -1.0f;
                out_dir[2] = -v;
                break;
            case 4: // PZ
                out_dir[0] = u;
                out_dir[1] = -v;
                out_dir[2] = 1.0f;
                break;
            case 5: // NZ
                out_dir[0] = -u;
                out_dir[1] = -v;
                out_dir[2] = -1.0f;
                break;
        }
    }


    static double surface_area(double x, double y)
    {
        return atan2(x * y, sqrt(x * x + y * y + 1.0));
    }



    void sh_integrate_cubemap(float** face_data,
                              unsigned int width,
                              unsigned int height,
                              unsigned int components_per_pixel,
                              ShChannel* out_channels)
    {
        // zero out coeffecients for accumulation
        for (int comp = 0; comp < components_per_pixel; ++comp)
        {
            for (int s = 0; s < SH_COUNT; ++s)
                out_channels[comp].coeffs[s] = 0.0;
        }

        for (int face = 0; face < 6; ++face)
        {
            for (int y = 0; y < height; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    // center each pixel
                    double px = (double)x + 0.5;
                    double py = (double)y + 0.5;
                    // normalize into [-1, 1] range
                    double u = 2.0 * (px / (double)width) - 1.0;
                    double v = 2.0 * (py / (double)height) - 1.0;

                    // calculate the solid angle
                    double d_x = 1.0 / (double)width;
                    double x0 = u - d_x;
                    double y0 = v - d_x;
                    double x1 = u + d_x;
                    double y1 = v + d_x;
                    double d_a = surface_area(x0, y0) - surface_area(x0, y1) - surface_area(x1, y0) + surface_area(x1, y1);

                    // find vector on unit sphere
                    double dir[3];
                    uv_to_cube(u, v, face, dir);
                    normalize(dir, 3);

                    size_t pixel_start = (x + y * width) * components_per_pixel;

                    for (int s = 0; s < SH_COUNT; ++s)
                    {
                        double sh_val = sh_eval_9(s, dir[0], dir[1], dir[2]);

                        for (int comp = 0; comp < components_per_pixel; ++comp)
                        {
                            double col = face_data[face][pixel_start + comp];
                            out_channels[comp].coeffs[s] += col * sh_val * d_a;
                        }
                    }
                }
            }
        }
    }

}



void grProcessSH9Extract(unsigned int irradianceTexID, // cube map tex
                         int texWidth,
                         int texHeight,
                         unsigned int components_per_pixel,
                         ShChannel* out_channels)
{
    int ret = 0;
    glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, &ret);
    int totalSize = texWidth * texHeight * 4;
    unsigned char* pixelsDataChar = static_cast<unsigned char*>(malloc(totalSize * sizeof(unsigned char)));
    float* pixelsDataFloatArray[6];
    GLuint tmpFbo;
    glGenFramebuffers(1, &tmpFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, tmpFbo);
    ShChannel SH9_RGBA[4];
    memset(&SH9_RGBA[0], 0, sizeof(SH9_RGBA));

//    int flipY = 1;
    for (unsigned int face = 0; face < 6; ++face) {
        float * pixelsDataFloat = static_cast<float *>(malloc(totalSize * sizeof(float)));
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,  irradianceTexID, 0);
        CHECK_GL_ERROR
        glReadPixels(0, 0, texWidth, texHeight, GL_RGBA, GL_FLOAT, pixelsDataFloat);
        CHECK_GL_ERROR
        for (unsigned int idx = 0; idx < totalSize; ++idx) {
            float c = pixelsDataFloat[idx];
            c = c > 1.0f ? 1.0f : c;
            pixelsDataChar[idx] = static_cast<unsigned char> ( c * 255.0f );
        }
        pixelsDataFloatArray[face] = pixelsDataFloat;
//        MiscUtils::dumpNamePNG(texWidth, texHeight, 4,  pixelsDataChar, 0, g_faceName[face]);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // SH9Convert
    sh_integrate_cubemap(pixelsDataFloatArray, texWidth, texHeight, 4, SH9_RGBA);

    for (int i = 0; i < components_per_pixel; ++i) {
        out_channels[i] = SH9_RGBA[i];
    }


    free(pixelsDataChar);
    for (int i = 0; i < 6; ++i) {
        free(pixelsDataFloatArray[i]);
    }
}

