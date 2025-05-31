package com.example.yuyaolong.yyalDemo.glSurfaceViewTests

import android.content.res.AssetManager

interface JNISimpleGLInterface {
    fun glSurfaceViewInit(assetManager: AssetManager)
    fun glSurfaceViewChanged(width: Int, height: Int)
    fun glSurfaceViewRender()
    fun glSurfaceViewDestroy()
}