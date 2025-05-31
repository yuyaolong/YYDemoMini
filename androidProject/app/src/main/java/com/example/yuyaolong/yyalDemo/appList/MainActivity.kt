package com.example.yuyaolong.yyalDemo.appList

import android.content.Intent
import android.net.Uri
import android.support.v7.app.AppCompatActivity
import android.os.Bundle
import android.os.Environment
import android.provider.Settings
import android.support.v4.app.ActivityCompat
import android.util.Log
import android.widget.ArrayAdapter
import android.widget.ListView
import com.example.yuyaolong.yyalDemo.Constants
import com.example.yuyaolong.yyalDemo.R
import com.example.yuyaolong.yyalDemo.foveation.FoveationActivity
import com.example.yuyaolong.yyalDemo.general3DRover.gaussianSplattingRover.GaussianSplattingRoverActivity
import com.example.yuyaolong.yyalDemo.lightCube.LightCubeActivity

class MainActivity : AppCompatActivity() {
    private var requestPrivilege = false
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.apps_layout)

        ActivityCompat.requestPermissions(this, arrayOf(android.Manifest.permission.WRITE_EXTERNAL_STORAGE,
            android.Manifest.permission.MANAGE_EXTERNAL_STORAGE,
            android.Manifest.permission.INTERNET), 1)

        val listView = findViewById<ListView>(R.id.appListView)
        val itemNames = arrayOf(
            "0. FoveationTest",
            "1. IBL",
            "2. GaussianSplattingTest")

        val arrayAdapter = ArrayAdapter(this, android.R.layout.simple_list_item_1, itemNames)
        listView.adapter = arrayAdapter


        listView.setOnItemClickListener { parent, view, position, id ->
            when (position) {
                0 -> startActivity(Intent(this, FoveationActivity::class.java))
                1 -> {
                    val intent = Intent(this, LightCubeActivity::class.java).apply {
                        putExtra(Constants.ACTIVITY_EXTRA_MESSAGE, Constants.IBL_TEST)
                    }
                    startActivity(intent)
                }
                2 -> {
                    val intent = Intent(this, GaussianSplattingRoverActivity::class.java).apply {
                        putExtra(Constants.ACTIVITY_EXTRA_MESSAGE, Constants.GaussianSplatting_TEST)
                    }
                    startActivity(intent)
                }
            }
        }
    }

    override fun onPostResume() {
        super.onPostResume()
        if (requestPrivilege) {
            if ((android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.R) &&
                Environment.isExternalStorageManager()
            ) {
                Log.d("file", "has write right")
            } else {
                val intent = Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION)
                intent.data = Uri.parse("package:$packageName")
                startActivityForResult(intent, 0)
            }
        }
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (requestCode == 0) {
            if ((android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.R) &&
                Environment.isExternalStorageManager()) {
                Log.d("file", "打开了 ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION 权限")
            } else {
                Log.d("file", "关闭了 ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION 权限")
            }
        }
    }
}