package com.example.yuyaolong.yyalDemo.lightCube

import android.content.Context
import android.opengl.GLSurfaceView
import android.content.res.AssetManager
import javax.microedition.khronos.opengles.GL10
import android.os.Bundle
import android.os.Handler
import android.os.Message
import android.util.Log
import com.example.yuyaolong.yyalDemo.Constants
import javax.microedition.khronos.egl.EGLConfig

class LightCubeRender(private val context: Context,
                      private val jniCalls: JNIARInterface) : GLSurfaceView.Renderer {

    private var frames = 0
    var stTime = System.nanoTime()

    // fps handler message
    private var fpsHandler: Handler? = null
    fun setFpsHandler(_handler: Handler?) {
        fpsHandler = _handler
    }

    override fun onSurfaceCreated(gl10: GL10, eglConfig: EGLConfig) {
        Log.d("yyal","surfaceCreated")
        jniCalls.arInit(context.assets)
    }

    override fun onSurfaceChanged(gl10: GL10, width: Int, height: Int) {
        Log.d("yyal","surfaceChanged")
        jniCalls.arResize(width, height)
    }

    override fun onDrawFrame(gl10: GL10) {
        Log.d("yyal","render draw frame")
        ++frames
        val curTime = System.nanoTime()
        if (curTime - stTime >= 800000000) {
            // send fps msg to update textview
            val fpsMsg = Message()
            fpsMsg.what = 0x123
            val fpsBundle = Bundle()
            fpsBundle.putString("FPS", "FPS: " + String.format("%.2f", frames / 8.0 * 10))
            fpsMsg.data = fpsBundle
            fpsHandler!!.sendMessage(fpsMsg)
            frames = 0
            stTime = curTime
        }
        jniCalls.arRender()
    }

    fun cleanRender() {
        Log.d("yyal","destroy render")
        jniCalls.arDestroy()
    }
}