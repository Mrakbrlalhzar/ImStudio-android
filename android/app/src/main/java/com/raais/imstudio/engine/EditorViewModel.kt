package com.raais.imstudio.engine
import androidx.lifecycle.ViewModel
import com.raais.imstudio.model.WidgetType

class EditorViewModel : ViewModel() {
    val state = ImStudioState()
    fun refresh() = state.refresh()
    fun select(id: Int?) { state.selectedId.value = id }
    fun addWidget(type: WidgetType) = state.addWidget(type.nativeName)
    fun generateCode(): String = state.generateCode()
}
