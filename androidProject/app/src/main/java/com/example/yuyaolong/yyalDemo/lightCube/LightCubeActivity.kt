package com.example.yuyaolong.yyalDemo.lightCube

import android.app.ActivityManager
import android.content.Context
import android.opengl.GLSurfaceView
import android.os.Bundle
import android.os.Handler
import android.os.Message
import android.support.v7.app.AppCompatActivity
import android.util.Log
import android.widget.SeekBar
import android.widget.TextView
import android.widget.ToggleButton
import com.example.yuyaolong.yyalDemo.Constants

import com.example.yuyaolong.yyalDemo.R

class LightCubeActivity : AppCompatActivity() {

    private val CONTEXT_CLIENT_VERSION = 3

    // OpenGL ES surface view
    private var mGLSurfaceView: GLSurfaceView? = null

    // NDK interface
    private var lightCubeRender :LightCubeRender? = null

    // UI
    private var fpsTextview: TextView? = null
    private var hdrBtn: ToggleButton? = null
    private var vrsToggleBtn: ToggleButton? = null
    private var lightSkb: SeekBar? = null
    private var hdrExpSkb: SeekBar? = null

    private var jniCalls: JNIARInterface? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        // set layout xml file
        setContentView(R.layout.activity_vrs_perf)
        // get layout handle

        val message = intent.getStringExtra(Constants.ACTIVITY_EXTRA_MESSAGE)
        if (message == Constants.PBR_TEST) {
            jniCalls = PBRTestInterface()
        } else if (message == Constants.IBL_TEST) {
            jniCalls = IBLTestInterface()
        }
        lightCubeRender = LightCubeRender(this, jniCalls!!)

        fpsTextview = findViewById(R.id.vrs_perf_fpsTextview)
        fpsTextview!!.text = "FPS: 00.00"
        val fpsHandler = object : Handler() {
            override fun handleMessage(msg: Message) {
                if (msg.what == 0x123) {
                    val data = msg.data.getString("FPS")
                    fpsTextview!!.text = data
                }
            }
        }
        lightCubeRender!!.setFpsHandler(fpsHandler)

        hdrBtn = findViewById(R.id.vrs_perf_hdrButton)
        hdrBtn!!.isChecked = false
        hdrBtn!!.text = "HDR ON"
        hdrBtn!!.textOff = "HDR OFF"
        hdrBtn!!.textOn = "HDR ON"
        hdrBtn!!.setOnClickListener {
            jniCalls!!.cubeEnableHDR(hdrBtn!!.isChecked)
            hdrExpSkb!!.isEnabled = hdrBtn!!.isChecked
        }

        vrsToggleBtn = findViewById(R.id.vrs_perf_enableVRS)
        vrsToggleBtn!!.isChecked = false
        vrsToggleBtn!!.text = "VRS ON"
        vrsToggleBtn!!.textOff = "VRS ON"
        vrsToggleBtn!!.textOn = "VRS OFF"
        vrsToggleBtn!!.setOnClickListener { jniCalls!!.cubeToggleVRS(vrsToggleBtn!!.isChecked) }


        lightSkb = findViewById(R.id.vrs_perf_lightSkb)
        lightSkb!!.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar, progress: Int, fromUser: Boolean) {
                // Log.d("yyal", "!!! bar value$progress")
                jniCalls!!.cubeUpdateLightValue(progress.toFloat())
            }

            override fun onStartTrackingTouch(seekBar: SeekBar) {

            }

            override fun onStopTrackingTouch(seekBar: SeekBar) {

            }
        })

        hdrExpSkb = findViewById(R.id.vrs_perf_hdrExpSkb)
        hdrExpSkb!!.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar, progress: Int, fromUser: Boolean) {
                jniCalls!!.cubeUpdateHDRExposure(progress * 1.0f / seekBar.max * 2) // 0 - 2, current 1
            }

            override fun onStartTrackingTouch(seekBar: SeekBar) {

            }

            override fun onStopTrackingTouch(seekBar: SeekBar) {

            }
        })
        hdrExpSkb!!.isEnabled = hdrBtn!!.isChecked

        mGLSurfaceView = findViewById(R.id.vrs_perf_glSurface)
        if (detectOpenGLES30()) {
            mGLSurfaceView!!.setEGLContextClientVersion(CONTEXT_CLIENT_VERSION)
            mGLSurfaceView!!.setRenderer(lightCubeRender)
        } else {
            Log.e("yyaolong", "Opengl es version small than 3.0")
            finish()
        }
        val gestureUtil =
            LightCubeGestureUtil(this, jniCalls!!)
        mGLSurfaceView!!.setOnTouchListener(gestureUtil.touchListener)
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
        lightCubeRender!!.cleanRender()
    }

    private fun detectOpenGLES30(): Boolean {
        val am = getSystemService(Context.ACTIVITY_SERVICE) as ActivityManager
        val info = am.deviceConfigurationInfo

        return info.reqGlEsVersion >= 0x30000
    }
}
