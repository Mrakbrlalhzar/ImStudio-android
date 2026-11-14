package com.raais.imstudio.engine
import android.util.Log
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.mutableStateOf
import com.raais.imstudio.NativeBridge
import com.raais.imstudio.model.ImportReport
import com.raais.imstudio.model.Widget
import kotlinx.serialization.json.Json

class ImStudioState {
    private val TAG = "ImStudioState"
    private val json = Json { ignoreUnknownKeys = true }
    val widgets = mutableStateListOf<Widget>()
    val selectedId = mutableStateOf<Int?>(null)
    val nativeError = mutableStateOf<String?>(null)
    val lastMessage = mutableStateOf<String?>(null)
    val lastImportReport = mutableStateOf<ImportReport?>(null)

    init {
        if (!NativeBridge.isLoaded) {
            nativeError.value = "Native lib not loaded: ${NativeBridge.loadError}"
            Log.e(TAG, nativeError.value!!)
        } else {
            try { NativeBridge.initBuffer(); Log.i(TAG, "initBuffer OK") }
            catch (t: Throwable) {
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

    fun clearAll(): Boolean {
        if (!NativeBridge.isLoaded) return false
        val ok = runCatching { NativeBridge.clearAll(); true }.getOrDefault(false)
        refresh()
        lastMessage.value = if (ok) "همه ویجت‌ها پاک شدند" else "پاک‌سازی ناموفق"
        return ok
    }

    fun loadFromText(text: String): Boolean {
        if (!NativeBridge.isLoaded) return false
        val ok = try { NativeBridge.loadFromText(text) } catch (t: Throwable) { Log.e(TAG, "loadFromText", t); false }
        refresh()
        lastMessage.value = if (ok) "پروژه بارگذاری شد (${widgets.size} ویجت)" else "فرمت پشتیبانی نمی‌شود"
        return ok
    }

    fun importFromCpp(code: String): Boolean {
        if (!NativeBridge.isLoaded) return false
        val ok = try { NativeBridge.importFromCpp(code) } catch (t: Throwable) { Log.e(TAG, "importFromCpp", t); false }
        refresh()
        lastMessage.value = if (ok) "کد C++ ایمپورت شد (${widgets.size} ویجت)"
            else "هیچ ویجت ImGui شناخته‌شده‌ای پیدا نشد"
        return ok
    }

    /** ★ Folder import — accepts list of (path, content) pairs sorted by caller */
    fun importFromFiles(files: List<Pair<String, String>>): ImportReport? {
        if (!NativeBridge.isLoaded) return null
        try {
            NativeBridge.importBegin()
            for ((path, content) in files) {
                NativeBridge.importFile(path, content)
            }
            val reportJson = NativeBridge.importEnd()
            val report = try { json.decodeFromString<ImportReport>(reportJson) }
                catch (t: Throwable) { Log.e(TAG, "report decode failed: $reportJson", t); ImportReport() }
            lastImportReport.value = report
            refresh()
            lastMessage.value = "ایمپورت: ${report.session.totalWidgets} ویجت از ${report.session.totalFiles} فایل"
            return report
        } catch (t: Throwable) {
            Log.e(TAG, "importFromFiles", t)
            lastMessage.value = "ایمپورت ناموفق: ${t.message}"
            return null
        }
    }

    fun dumpToJson(): String =
        if (!NativeBridge.isLoaded) "{\"version\":1,\"objects\":[]}"
        else try { NativeBridge.dumpToJson() } catch (t: Throwable) { "{\"version\":1,\"objects\":[]}" }

    fun dispose() { if (NativeBridge.isLoaded) runCatching { NativeBridge.shutdown() } }
}
