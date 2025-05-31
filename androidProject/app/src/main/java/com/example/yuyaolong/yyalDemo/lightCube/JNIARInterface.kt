package com.example.yuyaolong.yyalDemo.lightCube

import android.content.res.AssetManager

// AR for advanced rendering
interface JNIARInterface {
    fun arInit(assetManager: AssetManager)
    fun arResize(width: Int, height: Int)
    fun arRender()
    fun arDestroy()

    fun cubeScale(scaleFactor: Float)
    fun cubeScroll(dx: Float, dy: Float, endPx: Float, endPy: Float)
    fun cameraRotate(dx: Float, dy: Float, endPx: Float, endPy: Float)
    fun cameraScale(dx: Float, dy: Float, endPx: Float, endPy: Float)
    fun cubeReset()

    fun cubeEnableHDR(enableHDR: Boolean)
    fun cubeUpdateHDRExposure(exposure: Float)
    fun cubeUpdateLightValue(lightValue: Float)
    fun cubeToggleVRS(enableVRS: Boolean)
}