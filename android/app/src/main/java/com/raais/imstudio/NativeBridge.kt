package com.raais.imstudio
import android.util.Log

object NativeBridge {
    private const val TAG = "ImStudioNativeBridge"
    @Volatile var loadError: String? = null; private set
    @Volatile var isLoaded: Boolean = false; private set
    init {
        try {
            System.loadLibrary("imstudio_core")
            isLoaded = true
            Log.i(TAG, "lib loaded")
        } catch (t: Throwable) {
            loadError = "${t.javaClass.simpleName}: ${t.message}"
            Log.e(TAG, "load failed", t)
        }
    }
    external fun initCrashHandler(dir: String)
    external fun initBuffer()
    external fun shutdown()
    external fun createWidget(type: String): Int
    external fun deleteWidget(id: Int): Boolean
    external fun getWidgetCount(): Int
    external fun getWidgetId(index: Int): Int
    external fun getWidgetType(id: Int): String
    external fun getWidgetLabel(id: Int): String
    external fun getWidgetValueS(id: Int): String
    external fun getWidgetX(id: Int): Float
    external fun getWidgetY(id: Int): Float
    external fun getWidgetSizeX(id: Int): Float
    external fun getWidgetSizeY(id: Int): Float
    external fun getWidgetWidth(id: Int): Float
    external fun setWidgetLabel(id: Int, value: String): Boolean
    external fun setWidgetValueS(id: Int, value: String): Boolean
    external fun moveWidget(id: Int, x: Float, y: Float): Boolean
    external fun resizeWidget(id: Int, sx: Float, sy: Float): Boolean
    external fun setWidgetWidth(id: Int, w: Float): Boolean
    external fun generateCode(): String
}
