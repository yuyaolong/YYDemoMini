package com.example.yuyaolong.yyalDemo.general3DRover

import android.content.res.AssetManager
import android.view.Surface

interface Gen3DJNIInterface {
    fun renderInit(assetManager: AssetManager, surface: Surface)
    fun renderResize(width: Int, height: Int)
    fun renderDraw()
    fun renderDestroy()

    fun cameraZoom(scaleFactor: Float)
    fun cameraTranslate()
    fun cameraRotate()

}