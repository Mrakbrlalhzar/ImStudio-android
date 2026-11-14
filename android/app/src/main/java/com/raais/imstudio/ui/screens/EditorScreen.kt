package com.raais.imstudio.ui.screens

import android.content.Intent
import android.net.Uri
import android.widget.Toast
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.*
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material.icons.filled.Folder
import androidx.compose.material.icons.filled.FolderOpen
import androidx.compose.material.icons.filled.Input
import androidx.compose.material.icons.filled.Save
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
import androidx.documentfile.provider.DocumentFile
import com.raais.imstudio.engine.EditorViewModel
import com.raais.imstudio.model.WidgetType
import com.raais.imstudio.ui.components.*
import com.raais.imstudio.ui.dialogs.CodeDialog
import com.raais.imstudio.ui.dialogs.ConfirmDeleteDialog
import com.raais.imstudio.ui.dialogs.ImportReportDialog
import com.raais.imstudio.util.CrashLogger
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.File

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun EditorScreen(vm: EditorViewModel) {
    val state = vm.state
    val ctx = LocalContext.current
    val scope = rememberCoroutineScope()
    val nativeError = state.nativeError.value
    val lastMessage = state.lastMessage.value
    val lastReport = state.lastImportReport.value
    val crashFiles: List<File> = remember {
        try { CrashLogger.listCrashFiles(ctx) } catch (_: Throwable) { emptyList() }
    }
    var showCode by remember { mutableStateOf(false) }
    var showPalette by remember { mutableStateOf(false) }
    var showReport by remember { mutableStateOf(false) }
    var pendingDelete by remember { mutableStateOf<Int?>(null) }
    var pendingClear by remember { mutableStateOf(false) }
    var importRunning by remember { mutableStateOf(false) }

    val loadLauncher = rememberLauncherForActivityResult(ActivityResultContracts.OpenDocument()) { uri: Uri? ->
        uri ?: return@rememberLauncherForActivityResult
        try {
            val text = ctx.contentResolver.openInputStream(uri)?.use { it.bufferedReader().readText() } ?: ""
            val ok = vm.loadFromText(text)
            Toast.makeText(ctx, if (ok) "JSON بارگذاری شد" else "فرمت JSON نامعتبر", Toast.LENGTH_LONG).show()
        } catch (t: Throwable) { Toast.makeText(ctx, "خطا: ${t.message}", Toast.LENGTH_LONG).show() }
    }
    val saveLauncher = rememberLauncherForActivityResult(ActivityResultContracts.CreateDocument("application/json")) { uri: Uri? ->
        uri ?: return@rememberLauncherForActivityResult
        try {
            val json = vm.dumpToJson()
            ctx.contentResolver.openOutputStream(uri)?.use { it.write(json.toByteArray()) }
            Toast.makeText(ctx, "ذخیره شد", Toast.LENGTH_SHORT).show()
        } catch (t: Throwable) { Toast.makeText(ctx, "خطا: ${t.message}", Toast.LENGTH_LONG).show() }
    }
    val importCppLauncher = rememberLauncherForActivityResult(ActivityResultContracts.OpenDocument()) { uri: Uri? ->
        uri ?: return@rememberLauncherForActivityResult
        try {
            val text = ctx.contentResolver.openInputStream(uri)?.use { it.bufferedReader().readText() } ?: ""
            val ok = vm.importFromCpp(text)
            Toast.makeText(ctx, if (ok) "کد ایمپورت شد" else "ویجت ImGui پیدا نشد", Toast.LENGTH_LONG).show()
        } catch (t: Throwable) { Toast.makeText(ctx, "خطا: ${t.message}", Toast.LENGTH_LONG).show() }
    }
    // ★ Folder import
    val folderLauncher = rememberLauncherForActivityResult(ActivityResultContracts.OpenDocumentTree()) { uri: Uri? ->
        uri ?: return@rememberLauncherForActivityResult
        try {
            ctx.contentResolver.takePersistableUriPermission(
                uri, Intent.FLAG_GRANT_READ_URI_PERMISSION
            )
        } catch (_: Throwable) {}
        importRunning = true
        scope.launch {
            val result = withContext(Dispatchers.IO) {
                try {
                    val tree = DocumentFile.fromTreeUri(ctx, uri)
                        ?: return@withContext null
                    val out = mutableListOf<Pair<String, String>>()
                    tree.listFiles().forEach { f ->
                        if (!f.isFile) return@forEach
                        val name = f.name ?: return@forEach
                        val lower = name.lowercase()
                        val ok = lower.endsWith(".cpp") || lower.endsWith(".cc") ||
                                lower.endsWith(".cxx") || lower.endsWith(".h") ||
                                lower.endsWith(".hpp") || lower.endsWith(".inl")
                        if (!ok) return@forEach
                        try {
                            val content = ctx.contentResolver.openInputStream(f.uri)
                                ?.use { it.bufferedReader().readText() } ?: ""
                            out.add(name to content)
                        } catch (_: Throwable) {}
                    }
                    // Sort: .cpp first, then .h/.hpp, then alphabetic
                    out.sortWith(compareBy(
                        { pair ->
                            val n = pair.first.lowercase()
                            when {
                                n.endsWith(".cpp") || n.endsWith(".cc") || n.endsWith(".cxx") -> 0
                                n.endsWith(".hpp") || n.endsWith(".h") || n.endsWith(".inl") -> 1
                                else -> 2
                            }
                        },
                        { it.first.lowercase() }
                    ))
                    out
                } catch (t: Throwable) {
                    android.util.Log.e("FolderImport", "read failed", t)
                    null
                }
            }
            if (result.isNullOrEmpty()) {
                importRunning = false
                Toast.makeText(ctx, "فایلی برای ایمپورت پیدا نشد", Toast.LENGTH_LONG).show()
            } else {
                val report = vm.importFromFiles(result)
                importRunning = false
                if (report != null) {
                    showReport = true
                } else {
                    Toast.makeText(ctx, "ایمپورت ناموفق", Toast.LENGTH_LONG).show()
                }
            }
        }
    }

    LaunchedEffect(Unit) { vm.refresh() }

    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text(if (importRunning) "در حال ایمپورت..." else "ImStudio") },
                actions = {
                    if (!importRunning) {
                        IconButton(onClick = { folderLauncher.launch(null) }) {
                            Icon(Icons.Default.Folder, contentDescription = "ایمپورت فولدر")
                        }
                        IconButton(onClick = { importCppLauncher.launch(arrayOf("*/*")) }) {
                            Icon(Icons.Default.Input, contentDescription = "ایمپورت فایل C++")
                        }
                        IconButton(onClick = { loadLauncher.launch(arrayOf("*/*")) }) {
                            Icon(Icons.Default.FolderOpen, contentDescription = "بارگذاری JSON")
                        }
                        IconButton(onClick = { saveLauncher.launch("imstudio_project.json") }) {
                            Icon(Icons.Default.Save, contentDescription = "ذخیره")
                        }
                        IconButton(onClick = { showCode = true }) {
                            Icon(androidx.compose.material.icons.Icons.Default.Code, contentDescription = "کد")
                        }
                        IconButton(onClick = { pendingClear = true }) {
                            Icon(Icons.Default.Delete, contentDescription = "پاک‌سازی")
                        }
                    } else {
                        CircularProgressIndicator(
                            modifier = Modifier.size(24.dp),
                            strokeWidth = 2.dp
                        )
                    }
                }
            )
        },
        floatingActionButton = {
            if (!importRunning) FloatingActionButton(onClick = {
                if (nativeError == null) showPalette = true
            }) { Icon(Icons.Default.Add, contentDescription = "افزودن") }
        }
    ) { padding ->
        Column(modifier = Modifier.padding(padding).fillMaxSize()) {
            if (nativeError != null) {
                Surface(color = MaterialTheme.colorScheme.errorContainer) {
                    Column(Modifier.padding(16.dp)) {
                        Text("خطای native", style = MaterialTheme.typography.titleMedium)
                        Text(nativeError, style = MaterialTheme.typography.bodySmall)
                    }
                }
            }
            if (lastMessage != null) {
                Surface(color = MaterialTheme.colorScheme.secondaryContainer) {
                    Row(modifier = Modifier.padding(12.dp).fillMaxWidth(),
                        horizontalArrangement = Arrangement.SpaceBetween) {
                        Text(lastMessage, style = MaterialTheme.typography.bodySmall)
                        if (lastReport != null) {
                            TextButton(onClick = { showReport = true }) { Text("گزارش") }
                        }
                    }
                }
            }
            if (crashFiles.isNotEmpty()) {
                Surface(color = MaterialTheme.colorScheme.tertiaryContainer) {
                    Text("${crashFiles.size} فایل کرش ذخیره‌شده", Modifier.padding(12.dp),
                        style = MaterialTheme.typography.bodySmall)
                }
            }
            Row(modifier = Modifier.weight(1f).fillMaxWidth()) {
                Box(modifier = Modifier.weight(0.62f).fillMaxHeight()) {
                    DesignCanvas(state, onSelect = { vm.select(it) }, onLongPress = { pendingDelete = it })
                }
                Box(modifier = Modifier.weight(0.38f).fillMaxHeight()) {
                    PropertiesPanel(state)
                }
            }
        }
    }

    if (showPalette) ComponentPaletteSheet(
        onSelect = { t: WidgetType -> vm.addWidget(t); showPalette = false },
        onDismiss = { showPalette = false })
    if (showCode) CodeDialog(code = vm.generateCode(), onDismiss = { showCode = false })
    if (showReport) lastReport?.let { rep ->
        ImportReportDialog(report = rep, onDismiss = { showReport = false })
    }
    if (pendingDelete != null) {
        val id = pendingDelete!!
        ConfirmDeleteDialog(widgetId = id,
            onConfirm = { state.deleteWidget(id); pendingDelete = null },
            onDismiss = { pendingDelete = null })
    }
    if (pendingClear) {
        AlertDialog(onDismissRequest = { pendingClear = false },
            title = { Text("پاک‌سازی") },
            text = { Text("همه ویجت‌ها حذف شوند؟") },
            confirmButton = { TextButton(onClick = { vm.clearAll(); pendingClear = false }) { Text("پاک کن") } },
            dismissButton = { TextButton(onClick = { pendingClear = false }) { Text("لغو") } })
    }
}
