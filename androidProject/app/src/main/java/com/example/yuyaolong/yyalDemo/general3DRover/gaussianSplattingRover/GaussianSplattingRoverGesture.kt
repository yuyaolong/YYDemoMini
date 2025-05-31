package com.example.yuyaolong.yyalDemo.general3DRover.gaussianSplattingRover

import android.content.Context
import android.util.Log
import android.view.GestureDetector
import android.view.MotionEvent
import android.view.ScaleGestureDetector
import android.view.View

class GaussianSplattingRoverGesture(private val mContext: Context,
                                    private val mJniCalls: GaussianSplattingRoverJNICalls)
{
    private var mTouchListener: View.OnTouchListener
    private val mOneFingerDetector: GestureDetector
    private val mTwoFingerDetector: ScaleGestureDetector
    private var mThirdFingerPointerId = INVALID_POINTER_ID

    public fun getTouchListener(): View.OnTouchListener { return mTouchListener }

    private inner class OneFingerListener : GestureDetector.SimpleOnGestureListener() {
        override fun onDoubleTap(e: MotionEvent): Boolean {
            Log.d("yyal_touch", "one finger Double Tap!!!");
            return true
        }

        override fun onScroll(
            e1: MotionEvent,
            e2: MotionEvent,
            distanceX: Float,
            distanceY: Float
        ): Boolean {
            if (e1.pointerCount == 1 && e2.pointerCount == 1) {
                Log.d("yyal_touch", "one finger Scroll happen---dX: $distanceX, dy: $distanceY")
                Log.d("yyal_touch", "one finger Scroll happen---X: " + e2.x + ", Y: " + e2.y)
                // mJniCalls.cubeScroll(distanceX, distanceY, e2.x, e2.y)
                // todo do camera translate here
            }
            return true
        }
    }

    private inner class TwoFingersListener : ScaleGestureDetector.SimpleOnScaleGestureListener() {
        override fun onScale(detector: ScaleGestureDetector): Boolean {
            Log.d("yyal_touch","two finger Scale: ${detector.scaleFactor}")
            mJniCalls.cameraZoom(detector.scaleFactor)
            return true
        }
    }

    private inner class MyTouchListener : View.OnTouchListener {
        private var mLastTouchX = 0f
        private var mLastTouchY = 0f
        override fun onTouch(view: View, motionEvent: MotionEvent): Boolean {
            mOneFingerDetector.onTouchEvent(motionEvent)
            mTwoFingerDetector.onTouchEvent(motionEvent)
            when (motionEvent.actionMasked) {
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
                            Log.d("yyal_touch", "two fingers Move dx: $dx; dy: $dy; x: $x; y: $y")
                        }
                        if (motionEvent.pointerCount == 3) {
                            Log.d("yyal_touch", "three fingers Move dx: $dx; dy: $dy; x: $x; y: $y")
                            // todo camera rotate here
                        }
                        if (motionEvent.pointerCount == 4) {
                            // mJniCalls.cameraScale(-dx, -dy, x, y)
                            Log.d("yyal_touch", "four fingers Move dx: $dx; dy: $dy; x: $x; y: $y")
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
        mOneFingerDetector = GestureDetector(mContext, OneFingerListener())
        mTwoFingerDetector = ScaleGestureDetector(mContext, TwoFingersListener())
        mTouchListener = MyTouchListener()
    }

}