package com.example.yuyaolong.yyalDemo.foveation

import android.content.Context
import android.content.res.AssetManager
import android.opengl.GLSurfaceView
import android.util.Log
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class FoveationGLSurfaceRender(private val context:Context,
                               private val glSurfaceView: GLSurfaceView) : GLSurfaceView.Renderer{
    public var showTextureColor = 0
    companion object {
        const val GL_TEXTURE_EXTERNAL_OES = 0x8D65
        init {
            System.loadLibrary("foveationTest")
        }
    }

    private external fun glNativeInit(assetManager: AssetManager)
    private external fun glNativeResize(width: Int, height: Int)
    private external fun glNativeRender(showTextureColor: Int)
    private external fun glDestroy()
    private external fun glUpdateFoveation(msg: Int)

    fun updateFoveation(msg: Int) {
        glUpdateFoveation(msg)
    }

    fun resDestroy()
    {
        glDestroy()
    }

    override fun onSurfaceCreated(p0: GL10?, p1: EGLConfig?) {
        Log.d("yyal","surfaceCreated")
        glNativeInit(context.assets)
    }

    override fun onSurfaceChanged(p0: GL10?, p1: Int, p2: Int) {
        Log.d("yyal","surfaceChanged")
        glNativeResize(p1, p2)
    }

    override fun onDrawFrame(p0: GL10?) {
        glNativeRender(showTextureColor)
    }
}