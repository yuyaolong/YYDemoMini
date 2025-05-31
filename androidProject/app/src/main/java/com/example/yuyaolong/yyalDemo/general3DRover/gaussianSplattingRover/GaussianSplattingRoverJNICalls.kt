package com.example.yuyaolong.yyalDemo.general3DRover.gaussianSplattingRover

import android.content.res.AssetManager
import android.view.Surface
import com.example.yuyaolong.yyalDemo.general3DRover.Gen3DJNIInterface

class GaussianSplattingRoverJNICalls : Gen3DJNIInterface {
    companion object {
        init {
            System.loadLibrary("gaussianSplattingTest")
        }
    }

    private external fun renderInitNative(assetManager: AssetManager, surface: Surface)
    private external fun renderResizeNative(width: Int, height: Int)
    private external fun renderDrawNative()
    private external fun renderDestroyNative()
    private external fun cameraZoomNative(scaleFactor: Float)
    private external fun cameraTranslateNative()
    private external fun cameraRotateNative()

    override fun renderInit(assetManager: AssetManager, surface: Surface) {
        renderInitNative(assetManager, surface)
    }

    override fun renderResize(width: Int, height: Int) {
        renderResizeNative(width, height)
    }

    override fun renderDraw() {
        renderDrawNative()
    }

    override fun renderDestroy() {
        renderDestroyNative()
    }

    override fun cameraZoom(scaleFactor: Float) {
        cameraZoomNative(scaleFactor)
    }

    override fun cameraTranslate() {
        cameraTranslateNative()
    }

    override fun cameraRotate() {
        cameraRotateNative()
    }
}