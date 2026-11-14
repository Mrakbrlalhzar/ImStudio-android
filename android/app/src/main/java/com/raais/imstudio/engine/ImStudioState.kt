package com.raais.imstudio.engine
import android.util.Log
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.mutableStateOf
import com.raais.imstudio.NativeBridge
import com.raais.imstudio.model.Widget

class ImStudioState {
    private val TAG = "ImStudioState"
    val widgets = mutableStateListOf<Widget>()
    val selectedId = mutableStateOf<Int?>(null)
    val nativeError = mutableStateOf<String?>(null)

    init {
        if (!NativeBridge.isLoaded) {
            nativeError.value = "Native lib not loaded: ${NativeBridge.loadError}"
            Log.e(TAG, nativeError.value!!)
        } else {
            try {
                NativeBridge.initBuffer()
                Log.i(TAG, "initBuffer OK")
            } catch (t: Throwable) {
                nativeError.value = "initBuffer: ${t.javaClass.simpleName}: ${t.message}"
                Log.e(TAG, nativeError.value!!, t)
            }
        }
    }

    fun refresh() {
        if (nativeError.value != null) return
        try {
            val n = NativeBridge.getWidgetCount()
            val out = ArrayList<Widget>(n)
            for (i in 0 until n) {
                val id = NativeBridge.getWidgetId(i)
                if (id < 0) continue
                out += Widget(id, NativeBridge.getWidgetType(id),
                    NativeBridge.getWidgetLabel(id), NativeBridge.getWidgetValueS(id),
                    NativeBridge.getWidgetX(id), NativeBridge.getWidgetY(id),
                    NativeBridge.getWidgetSizeX(id), NativeBridge.getWidgetSizeY(id),
                    NativeBridge.getWidgetWidth(id))
            }
            widgets.clear(); widgets.addAll(out)
            if (selectedId.value != null && widgets.none { it.id == selectedId.value })
                selectedId.value = null
        } catch (t: Throwable) { Log.e(TAG, "refresh", t) }
    }

    fun addWidget(name: String) {
        if (nativeError.value != null) return
        try { val id = NativeBridge.createWidget(name); refresh(); if (id >= 0) selectedId.value = id }
        catch (t: Throwable) { Log.e(TAG, "addWidget", t) }
    }
    fun deleteWidget(id: Int) { runCatching { NativeBridge.deleteWidget(id) }; refresh() }
    fun moveWidget(id: Int, x: Float, y: Float) { runCatching { NativeBridge.moveWidget(id, x, y) }; refresh() }
    fun resizeWidget(id: Int, sx: Float, sy: Float) { runCatching { NativeBridge.resizeWidget(id, sx, sy) }; refresh() }
    fun setLabel(id: Int, v: String) { runCatching { NativeBridge.setWidgetLabel(id, v) }; refresh() }
    fun setValueS(id: Int, v: String) { runCatching { NativeBridge.setWidgetValueS(id, v) }; refresh() }
    fun generateCode(): String = try { NativeBridge.generateCode() } catch (t: Throwable) { "// error: ${t.message}" }
    fun selected(): Widget? = widgets.find { it.id == selectedId.value }
    fun dispose() { if (NativeBridge.isLoaded) runCatching { NativeBridge.shutdown() } }
}
