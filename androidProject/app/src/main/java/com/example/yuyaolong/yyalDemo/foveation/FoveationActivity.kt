package com.example.yuyaolong.yyalDemo.foveation

import android.app.ActivityManager
import android.content.Context
import android.graphics.Color
import android.opengl.GLSurfaceView
import android.os.Bundle
import android.support.v4.app.ActivityCompat
import android.support.v7.app.AppCompatActivity
import android.util.Log
import android.view.View
import android.widget.Button
import com.example.yuyaolong.yyalDemo.R

class FoveationActivity : AppCompatActivity() {
    private var mGLSurfaceView : GLSurfaceView? = null
    private val CONTEXT_CLIENT_VERSION = 3
    private var foveationGLSurfaceRender : FoveationGLSurfaceRender? = null
    private var settingFocalNo = 0

    private fun detectOpenGLES3(): Boolean {
        val am = getSystemService(Context.ACTIVITY_SERVICE) as ActivityManager
        val info = am.deviceConfigurationInfo

        return info.reqGlEsVersion >= 0x30000
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContentView(R.layout.activity_mem_layout)

        mGLSurfaceView = findViewById(R.id.mem_layout_glSurface)
        foveationGLSurfaceRender = FoveationGLSurfaceRender(this, mGLSurfaceView!!)
        if (detectOpenGLES3()) {
            mGLSurfaceView!!.setEGLContextClientVersion(CONTEXT_CLIENT_VERSION)
            mGLSurfaceView!!.setRenderer(foveationGLSurfaceRender)
        }

        findViewById<Button>(R.id.changeFocal)?.setOnClickListener {
            Log.d("BTN","changeFocal pushed")
            settingFocalNo = 1 - settingFocalNo
            foveationGLSurfaceRender!!.updateFoveation(11 + settingFocalNo)
            val btn = it as Button
            btn.text = "f" + settingFocalNo.toString()
            if (settingFocalNo == 1) {
                btn.setBackgroundColor(Color.parseColor("#E0E000"))
            } else if (settingFocalNo == 0) {
                btn.setBackgroundColor(Color.parseColor("#C0C0C0"))
            }

        }

        findViewById<Button>(R.id.fovAreaAdd)?.setOnClickListener {
            Log.d("BTN","A+ pushed")
            foveationGLSurfaceRender!!.updateFoveation(1)
        }

        findViewById<Button>(R.id.fovAreaMinus)?.setOnClickListener {
            Log.d("BTN","A- pushed")
            foveationGLSurfaceRender!!.updateFoveation(2)
        }

        findViewById<Button>(R.id.gainXAdd)?.setOnClickListener {
            Log.d("BTN","gx+ pushed")
            foveationGLSurfaceRender!!.updateFoveation(3)
        }

        findViewById<Button>(R.id.gainXMinus)?.setOnClickListener {
            Log.d("BTN","gx- pushed")
            foveationGLSurfaceRender!!.updateFoveation(4)
        }

        findViewById<Button>(R.id.gainYAdd)?.setOnClickListener {
            Log.d("BTN","gy+ pushed")
            foveationGLSurfaceRender!!.updateFoveation(5)
        }

        findViewById<Button>(R.id.gainYMinus)?.setOnClickListener {
            Log.d("BTN","gy- pushed")
            foveationGLSurfaceRender!!.updateFoveation(6)
        }

        findViewById<Button>(R.id.focalXAdd)?.setOnClickListener {
            Log.d("BTN","fx+ pushed")
            foveationGLSurfaceRender!!.updateFoveation(7)
        }

        findViewById<Button>(R.id.focalXMinus)?.setOnClickListener {
            Log.d("BTN","fx- pushed")
            foveationGLSurfaceRender!!.updateFoveation(8)
        }

        findViewById<Button>(R.id.focalYAdd)?.setOnClickListener {
            Log.d("BTN","fy+ pushed")
            foveationGLSurfaceRender!!.updateFoveation(9)
        }

        findViewById<Button>(R.id.focalYMinus)?.setOnClickListener {
            Log.d("BTN","fy- pushed")
            foveationGLSurfaceRender!!.updateFoveation(10)
        }

        findViewById<View>(R.id.mem_layout_glSurface)?.setOnClickListener {
            Log.d("BTN","surface pushed")
            foveationGLSurfaceRender!!.showTextureColor = 1 - foveationGLSurfaceRender!!.showTextureColor
        }

    }

    override fun onResume() {
        super.onResume()
        mGLSurfaceView!!.onResume()
    }

    override fun onPause() {
        super.onPause()
        mGLSurfaceView!!.onPause()
    }

    override fun onDestroy() {
        super.onDestroy()
        foveationGLSurfaceRender!!.resDestroy()
    }

}