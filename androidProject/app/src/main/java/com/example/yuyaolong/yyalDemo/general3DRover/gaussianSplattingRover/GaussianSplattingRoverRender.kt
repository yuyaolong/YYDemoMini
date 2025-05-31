package com.example.yuyaolong.yyalDemo.general3DRover.gaussianSplattingRover

import android.content.Context
import android.opengl.GLSurfaceView
import android.util.Log
import com.example.yuyaolong.yyalDemo.glSurfaceViewTests.GaussianSplattingJniInterface
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class GaussianSplattingRoverRender(private val mContext: Context,
                                   private val mJniCalls: GaussianSplattingRoverJNICalls,
                                   private val mGLSurfaceView: GLSurfaceView) : GLSurfaceView.Renderer {
    override fun onSurfaceCreated(p0: GL10?, p1: EGLConfig?) {
        mJniCalls.renderInit(mContext.assets, mGLSurfaceView.holder.surface)
    }

    override fun onSurfaceChanged(p0: GL10?, p1: Int, p2: Int) {
        mJniCalls.renderResize(p1, p2)
    }

    override fun onDrawFrame(p0: GL10?) {
        mJniCalls.renderDraw()
    }

    fun closeRender() {
        mJniCalls.renderDestroy()
    }

}