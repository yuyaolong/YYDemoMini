package com.example.yuyaolong.yyalDemo.lightCube


import android.app.Activity
import android.util.Log
import com.example.yuyaolong.yyalDemo.lightCube.JNIARInterface
import android.view.View.OnTouchListener
import android.view.GestureDetector
import android.view.ScaleGestureDetector
import com.example.yuyaolong.yyalDemo.lightCube.LightCubeGestureUtil
import android.view.GestureDetector.SimpleOnGestureListener
import android.view.MotionEvent
import android.view.ScaleGestureDetector.SimpleOnScaleGestureListener
import android.view.View

class LightCubeGestureUtil(activity: Activity?, jniCalls: JNIARInterface) {
    var touchListener: OnTouchListener
    private val mOneFingerDetector: GestureDetector
    private val mTwoFingerDetector: ScaleGestureDetector
    private val mJniCalls: JNIARInterface
    private var mThirdFingerPointerId = INVALID_POINTER_ID

    private inner class OneFingerListener : SimpleOnGestureListener() {
        override fun onDoubleTap(e: MotionEvent): Boolean {
            //Log.d("hehe", "Double Tap!!!");
            mJniCalls.cubeReset()
            mJniCalls.cubeScale(1f)
            return true
        }

        override fun onScroll(
            e1: MotionEvent,
            e2: MotionEvent,
            distanceX: Float,
            distanceY: Float
        ): Boolean {
            if (e1.pointerCount == 1 && e2.pointerCount == 1) {
//                Log.d("hehe", "onScroll happen---dX: $distanceX, dy: $distanceY")
//                Log.d("hehe", "onScroll happen---X: " + e2.x + ", Y: " + e2.y)
                //cubeMoveNative(distanceX, distanceY, e2.getX(), e2.getY());
                mJniCalls.cubeScroll(distanceX, distanceY, e2.x, e2.y)
            }
            return true
        }
    }

    private inner class TwoFingersListener : SimpleOnScaleGestureListener() {
        override fun onScale(detector: ScaleGestureDetector): Boolean {
            mJniCalls.cubeScale(detector.scaleFactor)
            return true
        }
    }

    private inner class MyTouchListener : OnTouchListener {
        private var mLastTouchX = 0f
        private var mLastTouchY = 0f
        private val firstInside = true
        override fun onTouch(view: View, motionEvent: MotionEvent): Boolean {
            mOneFingerDetector.onTouchEvent(motionEvent)
            mTwoFingerDetector.onTouchEvent(motionEvent)
            val action = motionEvent.actionMasked
            when (action) {
                MotionEvent.ACTION_DOWN -> {}
                MotionEvent.ACTION_POINTER_DOWN -> {
                    val index = motionEvent.actionIndex
                    if (motionEvent.pointerCount >= 2) {
                        mThirdFingerPointerId = motionEvent.getPointerId(index)
                        val x = motionEvent.getX(index)
                        val y = motionEvent.getY(index)
                        mLastTouchX = x
                        mLastTouchY = y
                    }
                }
                MotionEvent.ACTION_MOVE -> {
                    if (mThirdFingerPointerId != INVALID_POINTER_ID) {
                        val thirdPointIndex = motionEvent.findPointerIndex(mThirdFingerPointerId)
                        val x = motionEvent.getX(thirdPointIndex)
                        val y = motionEvent.getY(thirdPointIndex)
                        val dx = x - mLastTouchX
                        val dy = y - mLastTouchY
                        mLastTouchX = x
                        mLastTouchY = y
                        if (motionEvent.pointerCount == 2) {
                            Log.d("hehe1", "dx: $dx; dy: $dy; x: $x; y: $y")
                        }
                        if (motionEvent.pointerCount == 3) {
                            mJniCalls.cameraRotate(-dx, -dy, x, y)
                        }
                        if (motionEvent.pointerCount == 4) {
                            mJniCalls.cameraScale(-dx, -dy, x, y)
                        }
                    }
                }
                MotionEvent.ACTION_POINTER_UP -> {
                    mThirdFingerPointerId = INVALID_POINTER_ID
                }
                MotionEvent.ACTION_UP -> {
                    mThirdFingerPointerId = INVALID_POINTER_ID
                }
                MotionEvent.ACTION_CANCEL -> {
                    mThirdFingerPointerId = INVALID_POINTER_ID
                }
            }
            return true
        }
    }

    companion object {
        const val INVALID_POINTER_ID = -1
    }

    init {
        mOneFingerDetector = GestureDetector(activity, OneFingerListener())
        mTwoFingerDetector = ScaleGestureDetector(activity, TwoFingersListener())
        touchListener = MyTouchListener()
        mJniCalls = jniCalls
    }
}