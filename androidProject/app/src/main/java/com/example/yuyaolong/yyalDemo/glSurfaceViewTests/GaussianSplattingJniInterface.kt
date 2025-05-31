package com.example.yuyaolong.yyalDemo.glSurfaceViewTests

import android.content.res.AssetManager

class GaussianSplattingJniInterface : JNISimpleGLInterface {
    companion object {
        init {
            System.loadLibrary("gaussianSplattingTest")
        }
    }

    private external fun glNativeInit(assetManager: AssetManager)
    private external fun glNativeResize(width: Int, height: Int)
    private external fun glNativeRender()
    private external fun glDestroy()

    override fun glSurfaceViewInit(assetManager: AssetManager) {
        glNativeInit(assetManager)
    }

    override fun glSurfaceViewChanged(width: Int, height: Int) {
        glNativeResize(width, height)
    }

    override fun glSurfaceViewRender() {
        glNativeRender()
    }

    override fun glSurfaceViewDestroy() {
        glDestroy()
    }
}