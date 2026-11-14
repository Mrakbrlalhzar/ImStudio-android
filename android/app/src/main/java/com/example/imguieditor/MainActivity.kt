package com.example.imguieditor
import android.app.NativeActivity
import android.os.Bundle
import android.util.Log
class MainActivity : NativeActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        Log.i("ImGuiEngine", "NativeActivity created")
    }
}
