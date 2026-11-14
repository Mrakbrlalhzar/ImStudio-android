package com.raais.imstudio.model
import androidx.compose.runtime.Immutable
@Immutable
data class Widget(
    val id: Int, val type: String, val label: String, val valueS: String,
    val x: Float, val y: Float, val sizeX: Float, val sizeY: Float, val width: Float
) {
    val displayText: String get() = valueS.ifBlank { label.ifBlank { type } }
}
