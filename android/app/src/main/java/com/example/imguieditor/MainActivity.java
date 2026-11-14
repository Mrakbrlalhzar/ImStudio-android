package com.example.imguieditor;

import android.app.NativeActivity;
import android.os.Bundle;
import android.util.Log;

public class MainActivity extends NativeActivity {
    static {
        try {
            System.loadLibrary("imgui_editor");
            Log.i("ImGuiEditor", "libimgui_editor.so loaded");
        } catch (Throwable t) {
            Log.e("ImGuiEditor", "Failed to load native library", t);
        }
    }
}
