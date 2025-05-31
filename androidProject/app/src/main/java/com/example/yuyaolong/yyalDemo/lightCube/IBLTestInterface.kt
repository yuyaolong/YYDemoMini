package com.example.yuyaolong.yyalDemo.lightCube

import android.content.res.AssetManager
import kotlin.math.exp

class IBLTestInterface : JNIARInterface {
    companion object {
        init {
            System.loadLibrary("iblCube")
        }
    }

    private external fun cubeInit(assetManager: AssetManager)
    private external fun cubeRender()
    private external fun cubeResize(width: Int, height: Int)
    private external fun cubeClean()

    private external fun cubeScaleNative(scaleFactor: Float)
    private external fun cubeScrollNative(dx: Float, dy: Float, endPx: Float, endPy: Float)
    private external fun cameraRotateNative(dx: Float, dy: Float, endPx: Float, endPy: Float)
    private external fun cameraScaleNative(dx: Float, dy: Float, endPx: Float, endPy: Float)
    private external fun cubeResetNative()

    private external fun cubeEnableHDRNative(enableHDR: Boolean)
    private external fun cubeUpdateHDRExposureNative(exposure: Float)
    private external fun cubeUpdateLightValueNative(lightValue: Float)
    private external fun cubeToggleVRSNative(enableVRS: Boolean)



    override fun arInit(assetManager: AssetManager) {
        cubeInit(assetManager)
    }

    override fun arResize(width: Int, height: Int) {
        cubeResize(width, height)
    }

    override fun arRender() {
        cubeRender()
    }

    override fun arDestroy() {
        cubeClean()
    }

    override fun cubeScale(scaleFactor: Float) {
        cubeScaleNative(scaleFactor)
    }

    override fun cubeScroll(dx: Float, dy: Float, endPx: Float, endPy: Float) {
        cubeScrollNative(dx, dy, endPx, endPy)
    }

    override fun cameraRotate(dx: Float, dy: Float, endPx: Float, endPy: Float) {
        cameraRotateNative(dx, dy, endPx, endPy)
    }

    override fun cameraScale(dx: Float, dy: Float, endPx: Float, endPy: Float) {
        cameraScaleNative(dx, dy, endPx, endPy)
    }

    override fun cubeReset() {
        cubeResetNative()
    }

    override fun cubeEnableHDR(enableHDR: Boolean) {
        cubeEnableHDRNative(enableHDR)
    }

    override fun cubeUpdateHDRExposure(exposure: Float) {
        cubeUpdateHDRExposureNative(exposure)
    }

    override fun cubeUpdateLightValue(lightValue: Float) {
        cubeUpdateLightValueNative(lightValue)
    }

    override fun cubeToggleVRS(enableVRS: Boolean) {
        cubeToggleVRS(enableVRS)
    }


}