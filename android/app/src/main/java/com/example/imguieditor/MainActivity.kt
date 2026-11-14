package com.example.imguieditor

import android.app.NativeActivity
import android.os.Bundle
import android.util.Log
import android.view.inputmethod.InputMethodManager

class MainActivity : NativeActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        Log.i("ImGuiEngine", "NativeActivity created")
    }

    // 供 JNI 调用的方法：显示软键盘
    fun ShowAndroidKeyboard() {
        val imm = getSystemService(INPUT_METHOD_SERVICE) as InputMethodManager
        imm.showSoftInput(window.decorView, InputMethodManager.SHOW_IMPLICIT)
    }
}
