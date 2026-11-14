package com.raais.imstudio.engine
import androidx.lifecycle.ViewModel
import com.raais.imstudio.model.ImportReport
import com.raais.imstudio.model.WidgetType
class EditorViewModel : ViewModel() {
    val state = ImStudioState()
    fun refresh() = state.refresh()
    fun select(id: Int?) { state.selectedId.value = id }
    fun addWidget(type: WidgetType) = state.addWidget(type.nativeName)
    fun generateCode(): String = state.generateCode()
    fun clearAll() = state.clearAll()
    fun loadFromText(text: String) = state.loadFromText(text)
    fun importFromCpp(code: String) = state.importFromCpp(code)
    fun importFromFiles(files: List<Pair<String, String>>): ImportReport? = state.importFromFiles(files)
    fun dumpToJson() = state.dumpToJson()
    override fun onCleared() { super.onCleared(); state.dispose() }
}
