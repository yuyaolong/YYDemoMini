package com.example.yuyaolong.yyalDemo.general3DRover.gaussianSplattingRover

import android.app.ActivityManager
import android.content.Context
import android.opengl.GLSurfaceView
import android.os.Bundle
import android.support.v7.app.AppCompatActivity
import android.util.Log
import android.view.SurfaceHolder
import com.example.yuyaolong.yyalDemo.R

class GaussianSplattingRoverActivity : AppCompatActivity() {
    // OpenGL ES surface view
    private var mGLSurfaceView: GLSurfaceView? = null
    private var mJniCalls: GaussianSplattingRoverJNICalls? = null
    private var mRender: GaussianSplattingRoverRender? = null

    private fun detectOpenGLES30(): Boolean {
        val am = getSystemService(Context.ACTIVITY_SERVICE) as ActivityManager
        val info = am.deviceConfigurationInfo

        return info.reqGlEsVersion >= 0x30000
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_general_3d_rover)

        mJniCalls = GaussianSplattingRoverJNICalls()
        mGLSurfaceView = findViewById(R.id.general_3d_rover_glSurface)
        mRender = GaussianSplattingRoverRender(this, mJniCalls!!, mGLSurfaceView!!)

        if (detectOpenGLES30()) {
            mGLSurfaceView!!.setEGLContextClientVersion(3)
            mGLSurfaceView!!.setRenderer(mRender)
        } else {
            Log.e("yyal", "Opengl es version small than 3.0")
            finish()
        }

        // set gesture listener
        mGLSurfaceView!!.setOnTouchListener(GaussianSplattingRoverGesture(this, mJniCalls!!).getTouchListener())
    }

    override fun onResume() {
        super.onResume()
        mGLSurfaceView!!.onResume()
        // TODO need to think handle resume later
    }

    override fun onPause() {
        super.onPause()
        mGLSurfaceView!!.onPause()
        // TODO need to think handle pause later
    }
    override fun onDestroy() {
        super.onDestroy()
        mRender!!.closeRender()
    }


}