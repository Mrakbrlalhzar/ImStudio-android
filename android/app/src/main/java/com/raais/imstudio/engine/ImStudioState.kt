package com.raais.imstudio.engine
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.mutableStateOf
import com.raais.imstudio.NativeBridge
import com.raais.imstudio.model.Widget

class ImStudioState {
    val widgets = mutableStateListOf<Widget>()
    val selectedId = mutableStateOf<Int?>(null)
    init { NativeBridge.initBuffer() }

    fun refresh() {
        val n = NativeBridge.getWidgetCount()
        val out = ArrayList<Widget>(n)
        for (i in 0 until n) {
            val id = NativeBridge.getWidgetId(i)
            if (id < 0) continue
            out += Widget(
                id, NativeBridge.getWidgetType(id),
                NativeBridge.getWidgetLabel(id),
                NativeBridge.getWidgetValueS(id),
                NativeBridge.getWidgetX(id), NativeBridge.getWidgetY(id),
                NativeBridge.getWidgetSizeX(id), NativeBridge.getWidgetSizeY(id),
                NativeBridge.getWidgetWidth(id)
            )
        }
        widgets.clear(); widgets.addAll(out)
        if (selectedId.value != null && widgets.none { it.id == selectedId.value })
            selectedId.value = null
    }

    fun addWidget(nativeName: String) {
        val id = NativeBridge.createWidget(nativeName)
        refresh(); if (id >= 0) selectedId.value = id
    }
    fun deleteWidget(id: Int) { NativeBridge.deleteWidget(id); refresh() }
    fun moveWidget(id: Int, x: Float, y: Float) { NativeBridge.moveWidget(id, x, y); refresh() }
    fun resizeWidget(id: Int, sx: Float, sy: Float) { NativeBridge.resizeWidget(id, sx, sy); refresh() }
    fun setLabel(id: Int, v: String) { NativeBridge.setWidgetLabel(id, v); refresh() }
    fun setValueS(id: Int, v: String) { NativeBridge.setWidgetValueS(id, v); refresh() }
    fun generateCode(): String = NativeBridge.generateCode()
    fun selected(): Widget? = widgets.find { it.id == selectedId.value }
}
