package com.raais.imstudio

object NativeBridge {
    init { System.loadLibrary("imstudio_core") }

    external fun initBuffer()
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
